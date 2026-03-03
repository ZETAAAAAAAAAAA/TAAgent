"""
Resource information service for RenderDoc.
"""

import base64
import os
import struct

import renderdoc as rd

from ..utils import Parsers


class ResourceService:
    """Resource information service"""

    def __init__(self, ctx, invoke_fn):
        self.ctx = ctx
        self._invoke = invoke_fn

    def _find_texture_by_id(self, controller, resource_id):
        """Find texture by resource ID"""
        target_id = Parsers.extract_numeric_id(resource_id)
        for tex in controller.GetTextures():
            tex_id_str = str(tex.resourceId)
            tex_id = Parsers.extract_numeric_id(tex_id_str)
            if tex_id == target_id:
                return tex
        return None

    def _decode_bc4_block(self, data, offset):
        """Decode a single BC4 block (8 bytes) to 16 values"""
        import struct
        block = data[offset:offset+8]
        r0, r1 = block[0], block[1]
        
        # Decode indices
        indices = []
        for i in range(6):
            byte_val = block[2 + i]
            for j in range(8):
                idx = (byte_val >> (j * 3)) & 0x07
                indices.append(idx)
        
        # Generate palette
        palette = [0.0] * 8
        palette[0] = r0 / 255.0
        palette[1] = r1 / 255.0
        
        if r0 > r1:
            for i in range(2, 7):
                palette[i] = ((7 - i) * r0 + (i - 1) * r1) / 6.0 / 255.0
            palette[7] = 0.0
        else:
            for i in range(2, 6):
                palette[i] = ((5 - i) * r0 + (i - 1) * r1) / 5.0 / 255.0
            palette[6] = 0.0
            palette[7] = 1.0
        
        # Decode pixels
        values = []
        for i in range(16):
            values.append(palette[indices[i]] if indices[i] < 8 else 0.0)
        return values

    def _decode_bc4(self, data, width, height):
        """Decode BC4 compressed texture to BGRA"""
        bgra = bytearray(width * height * 4)
        blocks_x = (width + 3) // 4
        blocks_y = (height + 3) // 4
        
        for by in range(blocks_y):
            for bx in range(blocks_x):
                block_offset = (by * blocks_x + bx) * 8
                values = self._decode_bc4_block(data, block_offset)
                
                for py in range(4):
                    for px in range(4):
                        x = bx * 4 + px
                        y = by * 4 + py
                        if x < width and y < height:
                            idx = (y * width + x) * 4
                            val = int(values[py * 4 + px] * 255.0)
                            bgra[idx + 0] = val  # B
                            bgra[idx + 1] = val  # G
                            bgra[idx + 2] = val  # R
                            bgra[idx + 3] = 255  # A
        return bytes(bgra)

    def _decode_bc5(self, data, width, height):
        """Decode BC5 compressed texture (normal map) to BGRA"""
        bgra = bytearray(width * height * 4)
        blocks_x = (width + 3) // 4
        blocks_y = (height + 3) // 4
        
        for by in range(blocks_y):
            for bx in range(blocks_x):
                block_offset = (by * blocks_x + bx) * 16
                # BC5 = two BC4 blocks (R and G channels)
                r_values = self._decode_bc4_block(data, block_offset)
                g_values = self._decode_bc4_block(data, block_offset + 8)
                
                for py in range(4):
                    for px in range(4):
                        x = bx * 4 + px
                        y = by * 4 + py
                        if x < width and y < height:
                            idx = (y * width + x) * 4
                            # Normal map: R=X, G=Y, Z calculated
                            nx = r_values[py * 4 + px]
                            ny = g_values[py * 4 + px]
                            # Reconstruct Z: nx^2 + ny^2 + nz^2 = 1
                            nz = max(0.0, 1.0 - nx * nx - ny * ny) ** 0.5
                            
                            bgra[idx + 0] = int(nz * 255.0)  # B = Z
                            bgra[idx + 1] = int(ny * 255.0)  # G = Y
                            bgra[idx + 2] = int(nx * 255.0)  # R = X
                            bgra[idx + 3] = 255  # A
        return bytes(bgra)

    def _decode_bc_color(self, data, width, height, format_str):
        """Decode BC1/BC2/BC3/BC7 compressed textures"""
        # Simple fallback - return gray placeholder
        print("[save_texture_as_tga] BC color format %s not fully implemented, using placeholder" % format_str)
        bgra = bytearray(width * height * 4)
        for i in range(width * height):
            bgra[i * 4 + 0] = 128
            bgra[i * 4 + 1] = 128
            bgra[i * 4 + 2] = 128
            bgra[i * 4 + 3] = 255
        return bytes(bgra)

    def get_buffer_contents(self, resource_id, offset=0, length=0):
        """Get buffer data"""
        if not self.ctx.IsCaptureLoaded():
            raise ValueError("No capture loaded")

        result = {"data": None, "error": None}

        def callback(controller):
            # Parse resource ID
            try:
                rid = Parsers.parse_resource_id(resource_id)
            except Exception:
                result["error"] = "Invalid resource ID: %s" % resource_id
                return

            # Find buffer
            buf_desc = None
            for buf in controller.GetBuffers():
                if buf.resourceId == rid:
                    buf_desc = buf
                    break

            if not buf_desc:
                result["error"] = "Buffer not found: %s" % resource_id
                return

            # Get data
            actual_length = length if length > 0 else buf_desc.length
            data = controller.GetBufferData(rid, offset, actual_length)

            result["data"] = {
                "resource_id": resource_id,
                "length": len(data),
                "total_size": buf_desc.length,
                "offset": offset,
                "content_base64": base64.b64encode(data).decode("ascii"),
            }

        self._invoke(callback)

        if result["error"]:
            raise ValueError(result["error"])
        return result["data"]

    def get_texture_info(self, resource_id):
        """Get texture metadata"""
        if not self.ctx.IsCaptureLoaded():
            raise ValueError("No capture loaded")

        result = {"texture": None, "error": None}

        def callback(controller):
            try:
                tex_desc = self._find_texture_by_id(controller, resource_id)

                if not tex_desc:
                    result["error"] = "Texture not found: %s" % resource_id
                    return

                result["texture"] = {
                    "resource_id": resource_id,
                    "width": tex_desc.width,
                    "height": tex_desc.height,
                    "depth": tex_desc.depth,
                    "array_size": tex_desc.arraysize,
                    "mip_levels": tex_desc.mips,
                    "format": str(tex_desc.format.Name()),
                    "dimension": str(tex_desc.type),
                    "msaa_samples": tex_desc.msSamp,
                    "byte_size": tex_desc.byteSize,
                }
            except Exception as e:
                import traceback
                result["error"] = "Error: %s\n%s" % (str(e), traceback.format_exc())

        self._invoke(callback)

        if result["error"]:
            raise ValueError(result["error"])
        return result["texture"]

    def get_texture_data(self, resource_id, mip=0, slice=0, sample=0, depth_slice=None):
        """Get texture pixel data."""
        if not self.ctx.IsCaptureLoaded():
            raise ValueError("No capture loaded")

        result = {"data": None, "error": None}

        def callback(controller):
            tex_desc = self._find_texture_by_id(controller, resource_id)

            if not tex_desc:
                result["error"] = "Texture not found: %s" % resource_id
                return

            # Validate mip level
            if mip < 0 or mip >= tex_desc.mips:
                result["error"] = "Invalid mip level %d (texture has %d mips)" % (
                    mip,
                    tex_desc.mips,
                )
                return

            # Validate slice for array/cube textures
            max_slices = tex_desc.arraysize
            if tex_desc.cubemap:
                max_slices = tex_desc.arraysize * 6
            if slice < 0 or (max_slices > 1 and slice >= max_slices):
                result["error"] = "Invalid slice %d (texture has %d slices)" % (
                    slice,
                    max_slices,
                )
                return

            # Validate sample for MSAA
            if sample < 0 or (tex_desc.msSamp > 1 and sample >= tex_desc.msSamp):
                result["error"] = "Invalid sample %d (texture has %d samples)" % (
                    sample,
                    tex_desc.msSamp,
                )
                return

            # Calculate dimensions at this mip level
            mip_width = max(1, tex_desc.width >> mip)
            mip_height = max(1, tex_desc.height >> mip)
            mip_depth = max(1, tex_desc.depth >> mip)

            # Validate depth_slice for 3D textures
            is_3d = tex_desc.depth > 1
            if depth_slice is not None:
                if not is_3d:
                    result["error"] = "depth_slice can only be used with 3D textures"
                    return
                if depth_slice < 0 or depth_slice >= mip_depth:
                    result["error"] = "Invalid depth_slice %d (texture has %d depth at mip %d)" % (
                        depth_slice,
                        mip_depth,
                        mip,
                    )
                    return

            # Create subresource specification
            sub = rd.Subresource()
            sub.mip = mip
            sub.slice = slice
            sub.sample = sample

            # Get texture data
            try:
                data = controller.GetTextureData(tex_desc.resourceId, sub)
            except Exception as e:
                result["error"] = "Failed to get texture data: %s" % str(e)
                return

            # Extract depth slice for 3D textures if requested
            output_depth = mip_depth
            if is_3d and depth_slice is not None:
                total_size = len(data)
                bytes_per_slice = total_size // mip_depth
                slice_start = depth_slice * bytes_per_slice
                slice_end = slice_start + bytes_per_slice
                data = data[slice_start:slice_end]
                output_depth = 1

            result["data"] = {
                "resource_id": resource_id,
                "width": mip_width,
                "height": mip_height,
                "depth": output_depth,
                "mip": mip,
                "slice": slice,
                "sample": sample,
                "depth_slice": depth_slice,
                "format": str(tex_desc.format.Name()),
                "dimension": str(tex_desc.type),
                "is_3d": is_3d,
                "total_depth": mip_depth if is_3d else 1,
                "data_length": len(data),
                "content_base64": base64.b64encode(data).decode("ascii"),
            }

        self._invoke(callback)

        if result["error"]:
            raise ValueError(result["error"])
        return result["data"]

    def save_texture(self, resource_id, output_path, file_type="PNG", mip=-1, slice_index=-1):
        """
        Save texture data to various formats using RenderDoc's native SaveTexture API.
        
        Supports all texture formats including BC compressed formats (BC1-BC7).
        
        Args:
            resource_id: The resource ID of the texture
            output_path: Full path where to save the file
            file_type: Output format - "PNG", "TGA", "DDS", "JPG", "HDR", "BMP", "EXR"
            mip: Mip level to save (-1 for all mips, only works with DDS)
            slice_index: Array slice to save (-1 for all slices, only works with DDS)
        
        Returns:
            Dict with success status, file path, and metadata
        """
        print("[save_texture] Called with resource_id=%s, output_path=%s, file_type=%s" % (resource_id, output_path, file_type))
        
        if not self.ctx.IsCaptureLoaded():
            raise ValueError("No capture loaded")

        # Map file type string to RenderDoc enum
        file_type_map = {
            "PNG": rd.FileType.PNG,
            "TGA": rd.FileType.TGA,
            "DDS": rd.FileType.DDS,
            "JPG": rd.FileType.JPG,
            "HDR": rd.FileType.HDR,
            "BMP": rd.FileType.BMP,
            "EXR": rd.FileType.EXR,
        }
        
        file_type_upper = file_type.upper()
        if file_type_upper not in file_type_map:
            raise ValueError("Unsupported file type: %s. Supported types: %s" % (file_type, ", ".join(file_type_map.keys())))
        
        rd_file_type = file_type_map[file_type_upper]

        result = {"data": None, "error": None}

        def callback(controller):
            try:
                print("[save_texture] Callback started")
                tex_desc = self._find_texture_by_id(controller, resource_id)

                if not tex_desc:
                    result["error"] = "Texture not found: %s" % resource_id
                    print("[save_texture] ERROR: %s" % result["error"])
                    return

                format_str = str(tex_desc.format.Name())
                width = tex_desc.width
                height = tex_desc.height
                print("[save_texture] Found texture: %dx%d, format=%s" % (width, height, format_str))
                
                # Ensure output directory exists
                output_dir = os.path.dirname(output_path)
                if output_dir:
                    os.makedirs(output_dir, exist_ok=True)
                
                # Use RenderDoc's native SaveTexture API
                texsave = rd.TextureSave()
                texsave.resourceId = tex_desc.resourceId
                texsave.mip = mip
                texsave.slice.sliceIndex = slice_index
                texsave.destType = rd_file_type
                texsave.alpha = rd.AlphaMapping.Preserve
                
                # Build output path with correct extension
                final_path = output_path
                ext_map = {"PNG": ".png", "TGA": ".tga", "DDS": ".dds", "JPG": ".jpg", "HDR": ".hdr", "BMP": ".bmp", "EXR": ".exr"}
                expected_ext = ext_map[file_type_upper]
                if not final_path.lower().endswith(expected_ext):
                    final_path += expected_ext
                
                print("[save_texture] Using native SaveTexture API for %s" % final_path)
                controller.SaveTexture(texsave, final_path)
                
                if os.path.exists(final_path):
                    file_size = os.path.getsize(final_path)
                    result["data"] = {
                        "success": True,
                        "path": final_path,
                        "resource_id": resource_id,
                        "format": format_str,
                        "width": width,
                        "height": height,
                        "file_type": file_type_upper,
                        "file_size_bytes": file_size,
                    }
                    print("[save_texture] Success! File size: %d bytes" % file_size)
                else:
                    result["error"] = "Failed to save texture - file not created"
                    print("[save_texture] ERROR: File not created")
            
            except Exception as e:
                import traceback
                result["error"] = str(e)
                print("[save_texture] EXCEPTION: %s\n%s" % (str(e), traceback.format_exc()))

        print("[save_texture] Invoking callback...")
        self._invoke(callback)
        print("[save_texture] Callback completed, result=%s" % result)

        if result["error"]:
            raise ValueError(result["error"])
        return result["data"]

    # ==================== Mesh Export ====================

    def _unpack_vertex_data(self, fmt, data, offset):
        """
        Unpack vertex data according to ResourceFormat.
        Based on RenderDoc's decode_mesh.py example.
        """
        # We don't handle 'special' formats - typically bit-packed such as 10:10:10:2
        if fmt.Special():
            return None

        formatChars = {}
        #                                 012345678
        formatChars[rd.CompType.UInt]  = "xBHxIxxxL"
        formatChars[rd.CompType.SInt]  = "xbhxixxxl"
        formatChars[rd.CompType.Float] = "xxexfxxxd"  # only 2, 4 and 8 are valid

        # These types have identical decodes, but we might post-process them
        formatChars[rd.CompType.UNorm] = formatChars[rd.CompType.UInt]
        formatChars[rd.CompType.UScaled] = formatChars[rd.CompType.UInt]
        formatChars[rd.CompType.SNorm] = formatChars[rd.CompType.SInt]
        formatChars[rd.CompType.SScaled] = formatChars[rd.CompType.SInt]

        # We need to fetch compCount components
        vertexFormat = str(fmt.compCount) + formatChars[fmt.compType][fmt.compByteWidth]

        # Unpack the data
        try:
            value = list(struct.unpack_from(vertexFormat, data, offset))
        except struct.error:
            return None

        # If the format needs post-processing such as normalisation, do that now
        if fmt.compType == rd.CompType.UNorm:
            divisor = float((2 ** (fmt.compByteWidth * 8)) - 1)
            value = [float(i) / divisor for i in value]
        elif fmt.compType == rd.CompType.SNorm:
            maxNeg = -float(2 ** (fmt.compByteWidth * 8)) / 2
            divisor = float(-(maxNeg - 1))
            value = [(float(i) if (i == maxNeg) else (float(i) / divisor)) for i in value]

        # If the format is BGRA, swap the two components
        if fmt.BGRAOrder():
            value = [value[i] for i in [2, 1, 0, 3]]

        return value

    def _get_indices(self, controller, draw, ib):
        """Get index data for a draw call"""
        # If the draw doesn't use an index buffer, generate sequential indices
        if not (draw.flags & rd.ActionFlags.Indexed):
            return list(range(draw.numIndices))

        # Get the character for the width of index
        indexFormat = 'B'
        if ib.byteStride == 2:
            indexFormat = 'H'
        elif ib.byteStride == 4:
            indexFormat = 'I'

        # Duplicate the format by the number of indices
        indexFormat = str(draw.numIndices) + indexFormat

        # Fetch the index buffer data
        ibdata = controller.GetBufferData(ib.resourceId, ib.byteOffset, 0)

        # Unpack all the indices, starting from the first index to fetch
        offset = draw.indexOffset * ib.byteStride
        
        try:
            indices = list(struct.unpack_from(indexFormat, ibdata, offset))
        except struct.error:
            # Fallback to sequential indices
            return list(range(draw.numIndices))

        # Apply the baseVertex offset
        return [i + draw.baseVertex for i in indices]

    def get_mesh_data(self, event_id, stage="vs_output"):
        """
        Get mesh data for a specific draw call.
        
        Args:
            event_id: The event ID of the draw call
            stage: "vs_input" for vertex shader inputs, "vs_output" for vertex shader outputs
        
        VS Output mode matches RenderDoc's Mesh Viewer CSV export format.
        """
        if not self.ctx.IsCaptureLoaded():
            raise ValueError("No capture loaded")

        result = {"data": None, "error": None}

        def callback(controller):
            try:
                # Find the draw call
                draw = self._find_draw_by_event_id(controller, event_id)
                if not draw:
                    result["error"] = "Draw call not found: %s" % event_id
                    return

                # Get pipeline state
                state = controller.GetPipelineState()
                
                if stage == "vs_output":
                    # Get VS Output data (post-transform vertices) - matches RenderDoc CSV export
                    vertex_data, indices, attr_names = self._get_vs_output_data(controller, draw, state)
                else:
                    # Get VS Input data (original vertex buffer data)
                    vertex_data, indices, attr_names = self._get_vs_input_data(controller, draw, state)

                if not vertex_data:
                    result["error"] = "No vertex data found"
                    return

                result["data"] = {
                    "event_id": event_id,
                    "draw_name": draw.GetName(controller.GetStructuredFile()),
                    "stage": stage,
                    "num_vertices": len(indices),
                    "num_indices": len(indices),
                    "indices": indices,
                    "attributes": attr_names,
                    "vertex_data": vertex_data
                }

            except Exception as e:
                import traceback
                result["error"] = str(e)
                traceback.print_exc()

        self._invoke(callback)

        if result["error"]:
            raise ValueError(result["error"])
        return result["data"]
    
    def _get_vs_input_data(self, controller, draw, state):
        """Get VS Input data (vertex shader inputs)"""
        # IMPORTANT: Must set frame event before getting mesh data
        controller.SetFrameEvent(draw.eventId, True)
        
        ib = state.GetIBuffer()
        vbs = state.GetVBuffers()
        attrs = state.GetVertexInputs()

        print("[_get_vs_input_data] eventId: %d" % draw.eventId)
        print("[_get_vs_input_data] attrs count: %d" % (len(attrs) if attrs else 0))
        print("[_get_vs_input_data] vbs count: %d" % (len(vbs) if vbs else 0))

        if not attrs:
            print("[_get_vs_input_data] No vertex inputs found!")
            return None, [], []

        # Get indices
        indices = self._get_indices(controller, draw, ib)
        
        # Build mesh input configuration for each attribute
        mesh_inputs = []
        for attr in attrs:
            if attr.perInstance:
                continue  # Skip instanced attributes
            
            if attr.vertexBuffer >= len(vbs):
                continue
                
            mesh_input = {
                'name': attr.name,
                'vertexResourceId': vbs[attr.vertexBuffer].resourceId,
                'vertexByteOffset': attr.byteOffset + vbs[attr.vertexBuffer].byteOffset,
                'vertexByteStride': vbs[attr.vertexBuffer].byteStride,
                'format': attr.format,
            }
            mesh_inputs.append(mesh_input)

        # Read vertex data for each attribute
        vertex_data = {}
        attr_values = {}
        
        for mesh_input in mesh_inputs:
            attr_name = mesh_input['name']
            fmt = mesh_input['format']
            
            # Read the vertex buffer
            vb_data = controller.GetBufferData(
                mesh_input['vertexResourceId'],
                mesh_input['vertexByteOffset'],
                0
            )
            
            # Calculate the maximum valid index based on buffer size
            vertex_stride = mesh_input['vertexByteStride']
            if vertex_stride > 0:
                max_valid_index = len(vb_data) // vertex_stride - 1
            else:
                max_valid_index = 0
            
            # Get unique indices to avoid reading same vertex multiple times
            unique_indices = sorted(set(indices))
            
            # Decode each unique vertex
            values_cache = {}
            for idx in unique_indices:
                # Clamp index to valid range
                clamped_idx = min(idx, max_valid_index)
                if clamped_idx < 0:
                    clamped_idx = 0
                offset = vertex_stride * clamped_idx
                value = self._unpack_vertex_data(fmt, vb_data, offset)
                if value is None:
                    value = [0.0] * fmt.compCount
                values_cache[idx] = value
            
            # Expand to all indices (with duplication for shared vertices)
            attr_values[attr_name] = [values_cache.get(idx, [0.0] * fmt.compCount) for idx in indices]
        
        # Map attributes to standard names
        for attr_name, values in attr_values.items():
            vertex_data[attr_name] = values

        return vertex_data, indices, list(vertex_data.keys())
    
    def _get_vs_output_data(self, controller, draw, state):
        """Get VS Output data (post-transform vertices) - matches RenderDoc Mesh Viewer CSV export"""
        # IMPORTANT: Must set frame event before getting PostVS data
        controller.SetFrameEvent(draw.eventId, True)
        
        # Get post-VS data
        postvs = controller.GetPostVSData(0, 0, rd.MeshDataStage.VSOut)
        
        if not postvs or postvs.vertexResourceId == rd.ResourceId.Null():
            # Fallback to VS Input if no VS Output available
            return self._get_vs_input_data(controller, draw, state)
        
        # Get indices from postvs
        indices = self._get_postvs_indices(controller, postvs, draw)
        
        # Get vertex shader reflection for output signature
        vs = state.GetShaderReflection(rd.ShaderStage.Vertex)
        if not vs:
            return None, [], []
        
        # Build mesh outputs from VS output signature (following RenderDoc's decode_mesh.py)
        mesh_outputs = []
        posidx = 0
        for i, attr in enumerate(vs.outputSignature):
            # Use semanticIdxName for semantic name (e.g., "TEXCOORD0", "SV_Position")
            name = attr.semanticIdxName if attr.semanticIdxName else (attr.varName if attr.varName else "UNKNOWN")
            mesh_output = {
                'name': name,
                'format': self._create_resource_format(attr.varType, attr.compCount),
                'vertexResourceId': postvs.vertexResourceId,
                'vertexByteStride': postvs.vertexByteStride,
                'compCount': attr.compCount,
            }
            
            # Track position index for shuffling
            if attr.systemValue == rd.ShaderBuiltin.Position:
                posidx = len(mesh_outputs)
            
            mesh_outputs.append(mesh_output)
        
        # Shuffle position to front (as RenderDoc does)
        if posidx > 0:
            pos = mesh_outputs[posidx]
            del mesh_outputs[posidx]
            mesh_outputs.insert(0, pos)
        
        # Calculate offsets for each output attribute (tightly packed)
        accum_offset = 0
        for mesh_out in mesh_outputs:
            mesh_out['vertexByteOffset'] = accum_offset
            fmt = mesh_out['format']
            comp_size = 8 if fmt.compByteWidth > 4 else 4
            accum_offset += comp_size * fmt.compCount
        
        # Read vertex data
        vertex_data = {}
        
        for mesh_out in mesh_outputs:
            attr_name = mesh_out['name']
            fmt = mesh_out['format']
            stride = mesh_out['vertexByteStride']
            attr_offset = mesh_out['vertexByteOffset']
            
            # Decode each vertex
            values = []
            for idx in indices:
                if idx < 0:
                    idx = 0
                
                byte_offset = attr_offset + stride * idx
                attr_size = fmt.compCount * (8 if fmt.compByteWidth > 4 else 4)
                data = controller.GetBufferData(mesh_out['vertexResourceId'], byte_offset, attr_size)
                
                value = self._unpack_vertex_data(fmt, data, 0)
                if value is None:
                    value = [0.0] * fmt.compCount
                values.append(list(value))
            
            vertex_data[attr_name] = values
        
        return vertex_data, indices, list(vertex_data.keys())
    
    def _create_resource_format(self, var_type, comp_count):
        """Create a ResourceFormat from variable type and component count"""
        fmt = rd.ResourceFormat()
        fmt.compByteWidth = rd.VarTypeByteSize(var_type)
        fmt.compCount = comp_count
        fmt.compType = rd.VarTypeCompType(var_type)
        fmt.type = rd.ResourceFormatType.Regular
        return fmt
    
    def _get_postvs_indices(self, controller, postvs, draw):
        """Get indices from post-VS data"""
        if postvs.indexResourceId == rd.ResourceId.Null():
            # No index buffer - generate sequential indices
            return list(range(postvs.numIndices))
        
        # Get index format character
        index_format = 'B'
        if postvs.indexByteStride == 2:
            index_format = 'H'
        elif postvs.indexByteStride == 4:
            index_format = 'I'
        
        # Duplicate the format by the number of indices
        format_str = str(postvs.numIndices) + index_format
        
        # Fetch the index buffer data
        ib_data = controller.GetBufferData(postvs.indexResourceId, postvs.indexByteOffset, 0)
        
        try:
            indices = list(struct.unpack_from(format_str, ib_data, 0))
        except struct.error:
            # Fallback to sequential indices
            return list(range(postvs.numIndices))
        
        # Apply baseVertex offset
        return [i + postvs.baseVertex for i in indices]

    def export_mesh_as_fbx(self, event_id, output_path, attribute_mapping, unit_scale=1, coordinate_system="ue", decode=None, buffer_config=None, flip_winding_order=False):
        """
        Export mesh data as FBX file.

        IMPORTANT: attribute_mapping MUST be provided by AI after analyzing CSV and DXBC.
        NO automatic detection is performed - all attribute mappings must be explicit.

        Args:
            event_id: The event ID of the draw call
            output_path: Full path where to save the FBX file
            attribute_mapping: REQUIRED dict mapping semantic names to attribute names.
                              Format: {"SEMANTIC": "stage:attribute_name"}
                              Stage options: "vs_input", "vs_output", "buffer:BufferName"
            unit_scale: Unit scale factor for FBX (1=cm, 100=m). Default is 1 (centimeters).
            coordinate_system: Target coordinate system. Options:
                              "ue" - Unreal Engine (Z-up, left-handed)
                              "unity" - Unity (Y-up, left-handed)
                              "blender" - Blender (Z-up, right-handed)
                              "maya" - Maya (Y-up, right-handed)
            decode: Optional dict of decode expressions for attributes.
            buffer_config: Optional dict configuring GPU Buffer data layout.
                          Required when using "buffer:BufferName" in attribute_mapping.
            flip_winding_order: If True, flip the winding order of triangle indices (swap index 1 and 2).
                               Use this when normals are inverted after import. Default is False.

        Returns:
            Dict with success status, file path, and mesh info
        """
        if not self.ctx.IsCaptureLoaded():
            raise ValueError("No capture loaded")

        result = {"data": None, "error": None}
        # Store attribute_mapping in a variable that won't be reassigned
        attr_mapping_input = attribute_mapping
        buffer_config_input = buffer_config

        def callback(controller):
            try:
                # Find the draw call
                draw = self._find_draw_by_event_id(controller, event_id)
                if not draw:
                    result["error"] = "Draw call not found: %s" % event_id
                    return

                # Get pipeline state
                state = controller.GetPipelineState()
                
                # Get BOTH VS Input and VS Output data to support mixed mappings
                vs_input_data, vs_input_indices, vs_input_attrs = self._get_vs_input_data(controller, draw, state)
                vs_output_data, vs_output_indices, vs_output_attrs = self._get_vs_output_data(controller, draw, state)
                
                # Store all data with stage prefix for easy access
                all_vertex_data = {
                    "vs_input": (vs_input_data, vs_input_attrs),
                    "vs_output": (vs_output_data, vs_output_attrs),
                }
                
                # Use indices from the stage that has position data (prefer vs_output for world space)
                indices = vs_output_indices if vs_output_data else vs_input_indices
                
                if not vs_input_data and not vs_output_data:
                    result["error"] = "No vertex data found"
                    return

                # ==================== Buffer Data Extraction ====================
                # Support "buffer:BufferName" format for GPU-driven rendering (UE5)
                buffer_data_cache = {}  # Cache for loaded buffer data
                
                def get_buffer_resource_id(buffer_name):
                    """Find buffer resource ID by name or index"""
                    # If buffer_config has explicit resource_id, use it
                    if buffer_config_input and buffer_name in buffer_config_input:
                        cfg = buffer_config_input[buffer_name]
                        if "resource_id" in cfg:
                            return cfg["resource_id"]
                    
                    # Try to find by name/index in shader resources
                    # Buffer names like "Buffer0", "Buffer1" map to t0, t1 slots
                    try:
                        import re
                        match = re.search(r'Buffer(\d+)', buffer_name)
                        if match:
                            slot = int(match.group(1))
                            # Get SRVs from pipeline state
                            srvs = state.GetSRVs()
                            if slot < len(srvs) and srvs[slot].resourceId != rd.ResourceId.Null():
                                return str(srvs[slot].resourceId)
                    except Exception as e:
                        print("[export_mesh_as_fbx] Error finding buffer: %s" % e)
                    
                    return None
                
                def load_buffer_data(buffer_name):
                    """Load and cache buffer data"""
                    if buffer_name in buffer_data_cache:
                        return buffer_data_cache[buffer_name]
                    
                    resource_id = get_buffer_resource_id(buffer_name)
                    if not resource_id:
                        return None
                    
                    # Find buffer
                    rid = Parsers.parse_resource_id(resource_id)
                    buf_desc = None
                    for buf in controller.GetBuffers():
                        if buf.resourceId == rid:
                            buf_desc = buf
                            break
                    
                    if not buf_desc:
                        return None
                    
                    # Load buffer data
                    data = controller.GetBufferData(rid, 0, 0)
                    buffer_data_cache[buffer_name] = {
                        "data": data,
                        "size": buf_desc.length,
                        "resource_id": resource_id
                    }
                    return buffer_data_cache[buffer_name]
                
                def get_buffer_attribute_data(buffer_name, offset, stride, format_str, num_vertices):
                    """Extract attribute data from buffer"""
                    buf_info = load_buffer_data(buffer_name)
                    if not buf_info:
                        return None
                    
                    data = buf_info["data"]
                    
                    # Determine format size
                    format_sizes = {
                        "float2": 8,
                        "float3": 12,
                        "float4": 16,
                    }
                    elem_size = format_sizes.get(format_str, 16)
                    
                    # Extract data for each vertex
                    values = []
                    for i in range(num_vertices):
                        byte_offset = offset + stride * i
                        if byte_offset + elem_size > len(data):
                            break
                        
                        # Unpack floats
                        if format_str == "float2":
                            val = list(struct.unpack_from('ff', data, byte_offset))
                        elif format_str == "float3":
                            val = list(struct.unpack_from('fff', data, byte_offset))
                        else:  # float4
                            val = list(struct.unpack_from('ffff', data, byte_offset))
                        values.append(val)
                    
                    return values

                # ==================== Decode Expression Parser ====================
                import math as _math
                
                def apply_decode_expression(vec, expr):
                    """
                    Apply decode expression to a vector.
                    vec: [x, y, z] or [x, y, z, w]
                    expr: expression string like "itof(x) / 32768" or "x * 2 - 1"
                    """
                    if not expr or expr == "none":
                        return vec
                    
                    expr = expr.strip()
                    
                    # Handle normalize wrapper
                    if expr.startswith("normalize(") and expr.endswith(")"):
                        inner_expr = expr[10:-1]
                        decoded = apply_decode_expression(vec, inner_expr)
                        length = _math.sqrt(sum(v * v for v in decoded[:3]))
                        if length > 0:
                            return [v / length for v in decoded[:3]] + decoded[3:]
                        return decoded
                    
                    # Handle itof(x) - integer to float
                    if expr.startswith("itof(") and expr.endswith(")"):
                        inner_expr = expr[5:-1]
                        decoded = [float(v) for v in vec]
                        return apply_decode_expression(decoded, inner_expr)
                    
                    # Handle itof(x) followed by operations
                    if "itof(x)" in expr:
                        decoded = [float(v) for v in vec]
                        remaining_expr = expr.replace("itof(x)", "x")
                        return apply_decode_expression(decoded, remaining_expr)
                    
                    # Handle simple scalar operations on x
                    import re
                    
                    pattern = r'x\s*([\+\-\*/])\s*(-?[\d.]+)'
                    matches = re.findall(pattern, expr)
                    
                    if matches:
                        res = list(vec)
                        for op, num_str in matches:
                            num = float(num_str)
                            if op == '*':
                                res = [v * num for v in res]
                            elif op == '/':
                                res = [v / num for v in res]
                            elif op == '+':
                                res = [v + num for v in res]
                            elif op == '-':
                                res = [v - num for v in res]
                        return res
                    
                    # Handle chained operations like "x * 2 - 1"
                    if expr.startswith("x"):
                        remaining = expr[1:].strip()
                        res = list(vec)
                        ops = re.findall(r'([\+\-\*/])\s*(-?[\d.]+)', remaining)
                        for op, num_str in ops:
                            num = float(num_str)
                            if op == '*':
                                res = [v * num for v in res]
                            elif op == '/':
                                res = [v / num for v in res]
                            elif op == '+':
                                res = [v + num for v in res]
                            elif op == '-':
                                res = [v - num for v in res]
                        return res
                    
                    return vec
                
                def apply_decode_to_attribute(data, expr):
                    """Apply decode expression to entire attribute data array."""
                    if not expr or expr == "none" or not data:
                        return data
                    return [apply_decode_expression(vec, expr) for vec in data]
                
                # Store decode expressions for later use
                decode_expressions = decode or {}

                def parse_mapping_value(value):
                    """Parse mapping value like 'vs_output:TEXCOORD9' or 'buffer:Buffer1'
                    Returns: (stage, attr_name) or (buffer_name, semantic_for_offset)
                    """
                    if isinstance(value, dict):
                        return value.get("stage"), value.get("attr")
                    if isinstance(value, str):
                        if ":" in value:
                            parts = value.split(":", 1)
                            stage_part = parts[0].lower().strip()
                            if stage_part in ("vs_input", "vs_output"):
                                return stage_part, parts[1].strip()
                            elif stage_part == "buffer":
                                # buffer:BufferName format
                                return "buffer", parts[1].strip()
                        raise ValueError("attribute_mapping must include stage prefix. Use 'vs_output:ATTR_NAME', 'vs_input:ATTR_NAME', or 'buffer:BufferName' format.")
                    return None, None

                # Build attribute mapping from AI-provided mapping ONLY
                if not attr_mapping_input:
                    result["error"] = "attribute_mapping is REQUIRED. AI must analyze CSV and DXBC first, then provide explicit mappings."
                    return
                
                final_mapping = {}  # Maps semantic to (stage, attr_name)
                buffer_mappings = {}  # Maps semantic to (buffer_name, offset_key)
                
                for key, value in attr_mapping_input.items():
                    stage_hint, attr_name = parse_mapping_value(value)
                    if not stage_hint or not attr_name:
                        result["error"] = "Invalid mapping for '%s': %s. Format must be 'stage:attr_name' or 'buffer:BufferName'" % (key, value)
                        return
                    
                    if stage_hint == "buffer":
                        # Buffer mapping - need to get offset from buffer_config
                        if not buffer_config_input or attr_name not in buffer_config_input:
                            result["error"] = "buffer_config is required for buffer mapping '%s'. Provide buffer_config with '%s' configuration." % (key, attr_name)
                            return
                        
                        final_mapping[key] = ("buffer", attr_name)
                        buffer_mappings[key] = attr_name
                    else:
                        # vs_input or vs_output mapping
                        # Verify attribute exists
                        data, attrs = all_vertex_data.get(stage_hint, (None, []))
                        if not data or attr_name not in attrs:
                            result["error"] = "Attribute '%s' not found in %s. Available: %s" % (attr_name, stage_hint, attrs)
                            return
                        
                        final_mapping[key] = (stage_hint, attr_name)
                
                # Verify POSITION is mapped
                if 'POSITION' not in final_mapping:
                    result["error"] = "POSITION must be mapped in attribute_mapping"
                    return
                
                # ==================== Load Buffer Data ====================
                # Pre-load buffer data for all buffer mappings
                buffer_attr_data = {}  # {buffer_name: {semantic: [values]}}
                num_vertices = len(indices)
                
                for semantic, buffer_name in buffer_mappings.items():
                    if buffer_name not in buffer_attr_data:
                        buffer_attr_data[buffer_name] = {}
                    
                    cfg = buffer_config_input.get(buffer_name, {})
                    stride = cfg.get("stride", 32)
                    format_str = cfg.get("format", "float4")
                    
                    # Determine offset based on semantic
                    offset_key = semantic.lower() + "_offset"
                    offset = cfg.get(offset_key, 0)
                    
                    # Special handling for NORMAL, TANGENT, UV, COLOR
                    if semantic == "NORMAL":
                        offset = cfg.get("normal_offset", 0)
                    elif semantic == "TANGENT":
                        offset = cfg.get("tangent_offset", 0)
                    elif semantic == "UV":
                        offset = cfg.get("uv_offset", 0)
                    elif semantic == "COLOR":
                        offset = cfg.get("color_offset", 0)
                    
                    attr_data = get_buffer_attribute_data(buffer_name, offset, stride, format_str, num_vertices)
                    if attr_data:
                        buffer_attr_data[buffer_name][semantic] = attr_data
                        print("[export_mesh_as_fbx] Loaded %d vertices from buffer %s, offset %d for %s" % (len(attr_data), buffer_name, offset, semantic))
                    else:
                        print("[export_mesh_as_fbx] WARNING: Failed to load buffer data for %s from %s" % (semantic, buffer_name))
                
                # ==================== Post-process Buffer Data ====================
                # Handle UE5 packed normal format: w < 0 means flip normal direction
                for buffer_name, sem_data in buffer_attr_data.items():
                    if "NORMAL" in sem_data:
                        normal_data = sem_data["NORMAL"]
                        # Check if normals have 4 components (packed format)
                        if normal_data and len(normal_data[0]) >= 4:
                            flipped_count = 0
                            for i, n in enumerate(normal_data):
                                w = n[3] if len(n) > 3 else 1.0
                                if w < 0:
                                    # Flip normal direction
                                    normal_data[i] = [-n[0], -n[1], -n[2]] + n[3:]
                                    flipped_count += 1
                            if flipped_count > 0:
                                print("[export_mesh_as_fbx] Flipped %d normals with w < 0 (UE5 packed format)" % flipped_count)
                
                # Helper to get vertex data by (stage, attr_name)
                def get_vertex_data(stage, attr_name):
                    if not stage or not attr_name:
                        return None
                    
                    if stage == "buffer":
                        # Get from buffer data
                        if attr_name in buffer_attr_data:
                            for semantic, data in buffer_attr_data[attr_name].items():
                                return data
                        return None
                    
                    data, attrs = all_vertex_data.get(stage, (None, []))
                    if data and attr_name in data:
                        return data[attr_name]
                    return None
                
                # Update get_vertex_data to handle buffer semantics
                def get_vertex_data_by_mapping(semantic):
                    """Get vertex data for a semantic using the mapping"""
                    if semantic not in final_mapping:
                        return None
                    
                    stage, attr_name = final_mapping[semantic]
                    
                    if stage == "buffer":
                        # Buffer mapping - get from buffer_attr_data
                        for buf_name, sem_data in buffer_attr_data.items():
                            if semantic in sem_data:
                                return sem_data[semantic]
                        return None
                    else:
                        data, attrs = all_vertex_data.get(stage, (None, []))
                        if data and attr_name in data:
                            return data[attr_name]
                        return None
                
                # Handle clip space position (SV_Position) - apply perspective division
                position_info = final_mapping.get('POSITION')
                if position_info and position_info[0] != "buffer":
                    pos_data = get_vertex_data(position_info[0], position_info[1])
                    if pos_data and len(pos_data[0]) >= 4:
                        w = pos_data[0][3]
                        if abs(w - 1.0) > 0.01 and w != 0:
                            print("[export_mesh_as_fbx] WARNING: SV_Position is in clip space. Applying perspective division.")
                            new_pos_data = []
                            for pos in pos_data:
                                w = pos[3] if len(pos) >= 4 else 1.0
                                if w != 0:
                                    new_pos = [pos[0]/w, pos[1]/w, pos[2]/w]
                                else:
                                    new_pos = pos[:3]
                                new_pos_data.append(new_pos)
                            all_vertex_data[position_info[0]][0][position_info[1]] = new_pos_data
                            result["clip_space_warning"] = "SV_Position was in clip space, applied perspective division."

                # ==================== Apply Decode Expressions ====================
                decode_info = {}
                for semantic, expr in decode_expressions.items():
                    if not expr or expr == "none":
                        continue
                    
                    attr_info = final_mapping.get(semantic)
                    if not attr_info:
                        continue
                    
                    stage_name, attr_name = attr_info
                    
                    if stage_name == "buffer":
                        # Decode buffer data
                        if attr_name in buffer_attr_data and semantic in buffer_attr_data[attr_name]:
                            original_data = buffer_attr_data[attr_name][semantic]
                            decoded_data = apply_decode_to_attribute(original_data, expr)
                            buffer_attr_data[attr_name][semantic] = decoded_data
                            decode_info[semantic] = expr
                            print("[export_mesh_as_fbx] Applied decode '%s' to buffer:%s for %s" % (expr, attr_name, semantic))
                    else:
                        # Decode vs_input/vs_output data
                        data, attrs = all_vertex_data.get(stage_name, (None, []))
                        if not data or attr_name not in data:
                            continue
                        
                        original_data = data[attr_name]
                        decoded_data = apply_decode_to_attribute(original_data, expr)
                        data[attr_name] = decoded_data
                        decode_info[semantic] = expr
                        print("[export_mesh_as_fbx] Applied decode '%s' to %s:%s" % (expr, stage_name, attr_name))

                # Create extended all_vertex_data with buffer data for FBX generation
                extended_vertex_data = {
                    "vs_input": all_vertex_data.get("vs_input", (None, [])),
                    "vs_output": all_vertex_data.get("vs_output", (None, [])),
                    "buffer": ({}, [])  # Buffer data stored here
                }
                
                # Add buffer data to extended structure
                buffer_data_dict = {}
                buffer_attrs = []
                for buf_name, sem_data in buffer_attr_data.items():
                    for semantic, data in sem_data.items():
                        buffer_data_dict[semantic] = data
                        buffer_attrs.append(semantic)
                extended_vertex_data["buffer"] = (buffer_data_dict, buffer_attrs)

                # Generate FBX content
                fbx_path = self._generate_fbx_ascii_with_buffer(
                    output_path, indices, extended_vertex_data, final_mapping, unit_scale, coordinate_system, flip_winding_order
                )

                # Build attribute mapping info for result
                mapping_info = {}
                for semantic, (stage_name, attr_name) in final_mapping.items():
                    if stage_name == "buffer":
                        mapping_info[semantic] = "buffer:%s" % attr_name
                    else:
                        mapping_info[semantic] = "%s:%s" % (stage_name, attr_name)

                file_size = os.path.getsize(output_path)
                result["data"] = {
                    "success": True,
                    "path": output_path,
                    "event_id": event_id,
                    "unit_scale": unit_scale,
                    "coordinate_system": coordinate_system,
                    "num_vertices": len(indices),
                    "num_triangles": len(indices) // 3,
                    "attribute_mapping": mapping_info,
                    "decode": decode_info if decode_info else None,
                    "buffer_config": buffer_config_input if buffer_config_input else None,
                    "file_size_bytes": file_size
                }

            except Exception as e:
                import traceback
                result["error"] = str(e)
                traceback.print_exc()

        self._invoke(callback)

        if result["error"]:
            raise ValueError(result["error"])
        return result["data"]

    def _find_draw_by_event_id(self, controller, event_id):
        """Find a draw call by event ID"""
        target_id = int(event_id)
        
        def find_recursive(actions):
            for action in actions:
                if action.eventId == target_id:
                    return action
                if action.children:
                    found = find_recursive(action.children)
                    if found:
                        return found
            return None
        
        return find_recursive(controller.GetRootActions())

    def export_mesh_csv(self, event_id, output_path, stage="vs_output"):
        """
        Export mesh data as CSV file - matches RenderDoc Mesh Viewer CSV export format.
        
        Args:
            event_id: The event ID of the draw call
            output_path: Full path where to save the CSV file
            stage: "vs_input" for vertex shader inputs, "vs_output" for vertex shader outputs
        
        Returns:
            Dict with success status and file info
        """
        if not self.ctx.IsCaptureLoaded():
            raise ValueError("No capture loaded")

        result = {"data": None, "error": None}

        def callback(controller):
            try:
                import csv
                
                # Find the draw call
                draw = self._find_draw_by_event_id(controller, event_id)
                if not draw:
                    result["error"] = "Draw call not found: %s" % event_id
                    return

                # Get pipeline state
                state = controller.GetPipelineState()
                
                if stage == "vs_output":
                    vertex_data, indices, attr_names = self._get_vs_output_data(controller, draw, state)
                else:
                    vertex_data, indices, attr_names = self._get_vs_input_data(controller, draw, state)

                if not vertex_data:
                    result["error"] = "No vertex data found"
                    return

                # Build CSV header - expand each attribute by its component count
                header = ["VTX", "IDX"]
                component_counts = {}
                
                # Get component count for each attribute from first value
                for attr_name in attr_names:
                    if vertex_data[attr_name]:
                        comp_count = len(vertex_data[attr_name][0])
                    else:
                        comp_count = 4  # default
                    component_counts[attr_name] = comp_count
                    
                    # Add component columns
                    for c in range(comp_count):
                        header.append("%s.%s" % (attr_name, "xyzw"[c] if c < 4 else str(c)))
                
                # Write CSV
                output_dir = os.path.dirname(output_path)
                if output_dir:
                    os.makedirs(output_dir, exist_ok=True)
                
                with open(output_path, 'w', newline='', encoding='utf-8') as f:
                    writer = csv.writer(f)
                    writer.writerow(header)
                    
                    num_vertices = len(indices)
                    for i in range(num_vertices):
                        row = [i, indices[i]]  # VTX, IDX
                        for attr_name in attr_names:
                            values = vertex_data[attr_name][i] if i < len(vertex_data[attr_name]) else []
                            comp_count = component_counts[attr_name]
                            for c in range(comp_count):
                                if c < len(values):
                                    val = values[c]
                                    # Format float values
                                    if isinstance(val, float):
                                        row.append("%.5g" % val)
                                    else:
                                        row.append(str(val))
                                else:
                                    row.append("0")
                        writer.writerow(row)
                
                file_size = os.path.getsize(output_path)
                result["data"] = {
                    "success": True,
                    "path": output_path,
                    "event_id": event_id,
                    "stage": stage,
                    "num_vertices": num_vertices,
                    "attributes": attr_names,
                    "file_size_bytes": file_size
                }

            except Exception as e:
                import traceback
                result["error"] = str(e)
                traceback.print_exc()

        self._invoke(callback)

        if result["error"]:
            raise ValueError(result["error"])
        return result["data"]

    def export_mesh_json(self, event_id, output_path=None, stage="vs_output"):
        """
        Export mesh data as JSON file for UE import.
        
        Args:
            event_id: The event ID of the draw call
            output_path: Optional path to save JSON file. If None, returns data only.
            stage: "vs_input" for vertex shader inputs, "vs_output" for vertex shader outputs
        
        Returns:
            Dict with mesh data (positions, normals, uvs, tangents, indices) or file info
        """
        if not self.ctx.IsCaptureLoaded():
            raise ValueError("No capture loaded")

        result = {"data": None, "error": None}

        def callback(controller):
            try:
                import json
                import math
                
                # Find the draw call
                draw = self._find_draw_by_event_id(controller, event_id)
                if not draw:
                    result["error"] = "Draw call not found: %s" % event_id
                    return

                # Get pipeline state
                state = controller.GetPipelineState()
                
                if stage == "vs_output":
                    # Get VS Output data (post-transform vertices) - matches RenderDoc CSV export
                    vertex_data, indices, attr_names = self._get_vs_output_data(controller, draw, state)
                else:
                    # Get VS Input data (original vertex buffer data)
                    vertex_data, indices, attr_names = self._get_vs_input_data(controller, draw, state)

                if not vertex_data:
                    result["error"] = "No vertex data found"
                    return

                # Helper to check and fix invalid values
                def sanitize_value(v):
                    if isinstance(v, (int, float)):
                        if math.isnan(v) or math.isinf(v):
                            return 0.0
                        return float(v)
                    return v

                def sanitize_list(lst):
                    return [sanitize_value(v) for v in lst]

                # Build mesh data structure
                mesh_data = {
                    "event_id": event_id,
                    "draw_name": draw.GetName(controller.GetStructuredFile()),
                    "stage": stage,
                    "num_vertices": len(indices),
                    "num_triangles": len(indices) // 3,
                    "attributes": attr_names,
                }

                # Extract positions - check for SV_Position (VS Output) or POSITION (VS Input)
                pos_data = None
                for name in attr_names:
                    name_upper = name.upper()
                    if 'SV_POSITION' in name_upper or 'POSITION' in name_upper or 'POS' in name_upper:
                        pos_data = vertex_data.get(name, [])
                        break
                
                if pos_data:
                    # Deduplicate and create indexed vertices
                    unique_vertices = {}
                    positions = []
                    normals = []
                    uvs = []
                    tangents = []
                    colors = []
                    new_indices = []
                    
                    for i, idx in enumerate(indices):
                        # Get position for this vertex
                        if i < len(pos_data):
                            pos = tuple(sanitize_list(pos_data[i][:3]))
                        else:
                            pos = (0.0, 0.0, 0.0)
                        
                        # Get other attributes
                        normal = None
                        uv = None
                        tangent = None
                        color = None
                        
                        for name in attr_names:
                            upper_name = name.upper()
                            data = vertex_data.get(name, [])
                            if i < len(data):
                                if 'NORMAL' in upper_name:
                                    normal = tuple(sanitize_list(data[i][:3]))
                                elif 'TANGENT' in upper_name:
                                    tangent = tuple(sanitize_list(data[i][:4] if len(data[i]) >= 4 else data[i][:3] + [1.0]))
                                elif 'TEXCOORD' in upper_name or 'UV' in upper_name:
                                    uv = tuple(sanitize_list(data[i][:2]))
                                elif 'COLOR' in upper_name:
                                    color = tuple(sanitize_list(data[i][:4] if len(data[i]) >= 4 else data[i]))
                        
                        # Create vertex key for deduplication
                        vertex_key = (pos, normal, uv, tangent, color)
                        
                        if vertex_key not in unique_vertices:
                            vertex_idx = len(positions)
                            unique_vertices[vertex_key] = vertex_idx
                            positions.append(list(pos))
                            normals.append(list(normal) if normal else [0.0, 0.0, 1.0])
                            uvs.append(list(uv) if uv else [0.0, 0.0])
                            tangents.append(list(tangent) if tangent else [1.0, 0.0, 0.0, 1.0])
                            colors.append(list(color) if color else [1.0, 1.0, 1.0, 1.0])
                        else:
                            vertex_idx = unique_vertices[vertex_key]
                        
                        new_indices.append(vertex_idx)
                    
                    mesh_data["positions"] = positions
                    mesh_data["normals"] = normals
                    mesh_data["uvs"] = uvs
                    mesh_data["tangents"] = tangents
                    mesh_data["colors"] = colors
                    mesh_data["indices"] = new_indices
                    mesh_data["num_unique_vertices"] = len(positions)
                else:
                    result["error"] = "No position data found"
                    return

                # Save to file if path provided
                if output_path:
                    output_dir = os.path.dirname(output_path)
                    if output_dir:
                        os.makedirs(output_dir, exist_ok=True)
                    
                    with open(output_path, 'w', encoding='utf-8') as f:
                        json.dump(mesh_data, f, indent=2)
                    
                    file_size = os.path.getsize(output_path)
                    # Return minimal info to avoid large JSON response
                    result["data"] = {
                        "success": True,
                        "path": output_path,
                        "event_id": event_id,
                        "stage": stage,
                        "num_vertices": len(indices),
                        "num_unique_vertices": len(positions),
                        "num_triangles": len(indices) // 3,
                        "attributes": attr_names,
                        "file_size_bytes": file_size
                    }
                else:
                    # Return full data if no output path
                    result["data"] = mesh_data

            except Exception as e:
                import traceback
                result["error"] = str(e)
                traceback.print_exc()

        self._invoke(callback)

        if result["error"]:
            raise ValueError(result["error"])
        return result["data"]

    def _get_indices(self, controller, draw, ib):
        """Get index buffer data"""
        import struct
        
        # Determine index format
        if draw.flags & rd.ActionFlags.Indexed:
            index_stride = ib.byteStride if ib.byteStride else 2
            index_format = 'H' if index_stride == 2 else 'I' if index_stride == 4 else 'B'
            
            # Get index buffer data
            ib_data = controller.GetBufferData(ib.resourceId, ib.byteOffset, 0)
            offset = draw.indexOffset * index_stride
            num_indices = draw.numIndices
            
            # Unpack indices
            format_str = str(num_indices) + index_format
            indices = struct.unpack_from(format_str, ib_data, offset)
            
            # Apply base vertex offset
            return [i + draw.baseVertex for i in indices]
        else:
            # No index buffer - generate sequential indices
            return list(range(draw.numIndices))

    def _get_vertex_attribute_data(self, controller, draw, attr, vbs, indices):
        """Get vertex attribute data for all vertices"""
        import struct
        
        if attr.vertexBuffer >= len(vbs):
            return None
        
        vb = vbs[attr.vertexBuffer]
        fmt = attr.format
        
        # Get format info
        comp_count = fmt.compCount
        comp_type = fmt.compType
        comp_width = fmt.compByteWidth
        
        # Determine struct format
        format_chars = {
            rd.CompType.UInt: 'BHI'[:comp_width * 2 + 1:2] if comp_width <= 4 else 'I',
            rd.CompType.SInt: 'bhi'[:comp_width * 2 + 1:2] if comp_width <= 4 else 'i',
            rd.CompType.Float: 'fe'[comp_width // 2 - 1] if comp_width in [2, 4, 8] else 'f',
            rd.CompType.UNorm: 'BHI'[:comp_width * 2 + 1:2] if comp_width <= 4 else 'I',
            rd.CompType.SNorm: 'bhi'[:comp_width * 2 + 1:2] if comp_width <= 4 else 'i',
        }
        
        char = format_chars.get(comp_type, 'f')
        vertex_format = str(comp_count) + char
        
        # Calculate total attribute size
        attr_size = comp_count * comp_width
        
        # Get unique vertices (deduplicate by index)
        unique_indices = sorted(set(indices))
        
        # Read vertex data
        vertex_values = {}
        vb_offset = vb.byteOffset + draw.vertexOffset * vb.byteStride + attr.byteOffset
        
        for idx in unique_indices:
            offset = vb_offset + idx * vb.byteStride
            data = controller.GetBufferData(vb.resourceId, offset, attr_size)
            
            try:
                values = list(struct.unpack_from(vertex_format, data, 0))
                
                # Normalize if needed
                if comp_type == rd.CompType.UNorm:
                    divisor = float((2 ** (comp_width * 8)) - 1)
                    values = [v / divisor for v in values]
                elif comp_type == rd.CompType.SNorm:
                    max_neg = -float(2 ** (comp_width * 8)) / 2
                    divisor = float(-(max_neg - 1))
                    values = [(v if v == max_neg else v / divisor) for v in values]
                
                vertex_values[idx] = values
            except:
                vertex_values[idx] = [0.0] * comp_count
        
        # Return values in index order
        return [vertex_values.get(idx, [0.0] * comp_count) for idx in indices]

    def _generate_fbx_ascii(self, output_path, indices, vertex_data, mapping, attr_names, unit_scale=1, coordinate_system="ue", flip_winding_order=False):
        """Generate FBX ASCII file content
        
        Args:
            unit_scale: Unit scale factor (1=cm, 100=m)
            coordinate_system: Target coordinate system ("ue", "unity", "blender", "maya")
            flip_winding_order: If True, flip triangle winding order (swap index 1 and 2)
        """
        from textwrap import dedent

        model_name = os.path.splitext(os.path.basename(output_path))[0]

        # Coordinate system settings for FBX
        # FBX axis convention: UpAxis, FrontAxis, CoordAxis
        # UpAxis: 0=X, 1=Y, 2=Z
        # FrontAxis: 0=ParityEven, 1=ParityOdd (determines handedness)
        # CoordAxis: 0=X, 1=Y, 2=Z
        COORD_SYSTEMS = {
            # UE: Z-up, left-handed (Y forward, X right)
            "ue": {
                "UpAxis": 2, "UpAxisSign": 1,
                "FrontAxis": 1, "FrontAxisSign": 1,
                "CoordAxis": 0, "CoordAxisSign": 1,
                "OriginalUpAxis": 2, "OriginalUpAxisSign": 1,
            },
            # Unity: Y-up, left-handed (Z forward, X right)
            "unity": {
                "UpAxis": 1, "UpAxisSign": 1,
                "FrontAxis": 2, "FrontAxisSign": 1,
                "CoordAxis": 0, "CoordAxisSign": 1,
                "OriginalUpAxis": 1, "OriginalUpAxisSign": 1,
            },
            # Blender: Z-up, right-handed (Y forward, -X right)
            "blender": {
                "UpAxis": 2, "UpAxisSign": 1,
                "FrontAxis": 1, "FrontAxisSign": 1,
                "CoordAxis": 0, "CoordAxisSign": -1,
                "OriginalUpAxis": 2, "OriginalUpAxisSign": 1,
            },
            # Maya: Y-up, right-handed (Z forward, -X right)
            "maya": {
                "UpAxis": 1, "UpAxisSign": 1,
                "FrontAxis": 2, "FrontAxisSign": 1,
                "CoordAxis": 0, "CoordAxisSign": -1,
                "OriginalUpAxis": 1, "OriginalUpAxisSign": 1,
            },
        }
        
        coord = COORD_SYSTEMS.get(coordinate_system.lower(), COORD_SYSTEMS["ue"])

        # Prepare index data (convert to FBX polygon format)
        min_idx = min(indices)
        idx_list = [idx - min_idx for idx in indices]
        
        # Flip winding order if requested (swap index 1 and 2 of each triangle)
        if flip_winding_order:
            flipped_idx_list = []
            for i in range(0, len(idx_list), 3):
                if i + 2 < len(idx_list):
                    # Original: (i0, i1, i2) -> Flipped: (i0, i2, i1)
                    flipped_idx_list.extend([idx_list[i], idx_list[i+2], idx_list[i+1]])
                else:
                    flipped_idx_list.extend(idx_list[i:])
            idx_list = flipped_idx_list
        
        polygons = [str(idx ^ -1 if i % 3 == 2 else idx) for i, idx in enumerate(idx_list)]

        # Get position data
        pos_attr = mapping.get('POSITION', 'POSITION')
        if pos_attr not in vertex_data:
            # Try to find any position attribute
            for name in attr_names:
                if 'POS' in name.upper():
                    pos_attr = name
                    break

        pos_data = vertex_data.get(pos_attr, [])

        # Deduplicate positions for vertices section
        unique_pos = {}
        for i, idx in enumerate(idx_list):
            if idx not in unique_pos and i < len(pos_data):
                unique_pos[idx] = pos_data[i]

        vertices = [str(v) for idx in sorted(unique_pos.keys()) for v in unique_pos.get(idx, [0, 0, 0])[:3]]
        
        # Build FBX template parts
        ARGS = {
            "model_name": model_name,
            "vertices_num": len(vertices),
            "vertices": ",".join(vertices),
            "polygons_num": len(polygons),
            "polygons": ",".join(polygons),
            "unit_scale": unit_scale,
            "coordinate_system": coordinate_system.upper(),
            "UpAxis": coord["UpAxis"],
            "UpAxisSign": coord["UpAxisSign"],
            "FrontAxis": coord["FrontAxis"],
            "FrontAxisSign": coord["FrontAxisSign"],
            "CoordAxis": coord["CoordAxis"],
            "CoordAxisSign": coord["CoordAxisSign"],
            "OriginalUpAxis": coord["OriginalUpAxis"],
            "OriginalUpAxisSign": coord["OriginalUpAxisSign"],
            "LayerElementNormal": "",
            "LayerElementNormalInsert": "",
            "LayerElementTangent": "",
            "LayerElementTangentInsert": "",
            "LayerElementUV": "",
            "LayerElementUVInsert": "",
            "LayerElementColor": "",
            "LayerElementColorInsert": "",
        }
        
        # Add normals
        norm_attr = mapping.get('NORMAL')
        if norm_attr and norm_attr in vertex_data:
            normals = [str(v) for values in vertex_data[norm_attr] for v in values[:3]]
            ARGS["LayerElementNormal"] = '''
                LayerElementNormal: 0 {
                    Version: 101
                    Name: ""
                    MappingInformationType: "ByPolygonVertex"
                    ReferenceInformationType: "Direct"
                    Normals: *%(normals_num)d {
                        a: %(normals)s
                    } 
                }
            ''' % {"normals": ",".join(normals), "normals_num": len(normals)}
            ARGS["LayerElementNormalInsert"] = '''
                LayerElement:  {
                    Type: "LayerElementNormal"
                    TypedIndex: 0
                }
            '''
        
        # Add UVs
        uv_attr = mapping.get('UV')
        if uv_attr and uv_attr in vertex_data:
            # Deduplicate UVs
            unique_uvs = {}
            for i, idx in enumerate(idx_list):
                if idx not in unique_uvs and i < len(vertex_data[uv_attr]):
                    unique_uvs[idx] = vertex_data[uv_attr][i]
            
            uvs = [str(1 - v if i else v) for idx in sorted(unique_uvs.keys()) 
                   for i, v in enumerate(unique_uvs.get(idx, [0, 0])[:2])]
            uv_indices = [str(idx) for idx in idx_list]
            
            ARGS["LayerElementUV"] = '''
                LayerElementUV: 0 {
                    Version: 101
                    Name: "map1"
                    MappingInformationType: "ByPolygonVertex"
                    ReferenceInformationType: "IndexToDirect"
                    UV: *%(uvs_num)d {
                        a: %(uvs)s
                    } 
                    UVIndex: *%(uv_indices_num)d {
                        a: %(uv_indices)s
                    } 
                }
            ''' % {"uvs": ",".join(uvs), "uvs_num": len(uvs), 
                   "uv_indices": ",".join(uv_indices), "uv_indices_num": len(uv_indices)}
            ARGS["LayerElementUVInsert"] = '''
                LayerElement:  {
                    Type: "LayerElementUV"
                    TypedIndex: 0
                }
            '''
        
        # Add tangents
        tan_attr = mapping.get('TANGENT')
        if tan_attr and tan_attr in vertex_data:
            tangents = []
            for values in vertex_data[tan_attr]:
                # Tangent is 4 components: xyz = tangent direction, w = binormal sign
                if len(values) >= 4:
                    tangents.extend([str(v) for v in values[:4]])
                elif len(values) >= 3:
                    tangents.extend([str(v) for v in values[:3]] + ["1.0"])
                else:
                    tangents.extend(["1.0", "0.0", "0.0", "1.0"])
            
            ARGS["LayerElementTangent"] = '''
                LayerElementTangent: 0 {
                    Version: 101
                    Name: ""
                    MappingInformationType: "ByPolygonVertex"
                    ReferenceInformationType: "Direct"
                    Tangents: *%(tangents_num)d {
                        a: %(tangents)s
                    } 
                }
            ''' % {"tangents": ",".join(tangents), "tangents_num": len(tangents)}
            ARGS["LayerElementTangentInsert"] = '''
                LayerElement:  {
                    Type: "LayerElementTangent"
                    TypedIndex: 0
                }
            '''
        
        # FBX ASCII template
        FBX_TEMPLATE = """
; FBX 7.3.0 project file
; Exported from RenderDoc MCP
; Coordinate system: %(coordinate_system)s

GlobalSettings:  {
    Version: 1000
    Properties70:  {
        P: "UpAxis", "int", "Integer", "",%(UpAxis)d
        P: "UpAxisSign", "int", "Integer", "",%(UpAxisSign)d
        P: "FrontAxis", "int", "Integer", "",%(FrontAxis)d
        P: "FrontAxisSign", "int", "Integer", "",%(FrontAxisSign)d
        P: "CoordAxis", "int", "Integer", "",%(CoordAxis)d
        P: "CoordAxisSign", "int", "Integer", "",%(CoordAxisSign)d
        P: "OriginalUpAxis", "int", "Integer", "",%(OriginalUpAxis)d
        P: "OriginalUpAxisSign", "int", "Integer", "",%(OriginalUpAxisSign)d
        P: "UnitScaleFactor", "double", "Number", "",%(unit_scale)d
        P: "OriginalUnitScaleFactor", "double", "Number", "",%(unit_scale)d
    }
}

Definitions:  {
    ObjectType: "Geometry" {
        Count: 1
        PropertyTemplate: "FbxMesh" {
            Properties70:  {
                P: "Primary Visibility", "bool", "", "",1
            }
        }
    }
    ObjectType: "Model" {
        Count: 1
        PropertyTemplate: "FbxNode" {
            Properties70:  {
                P: "Visibility", "Visibility", "", "A",1
            }
        }
    }
}

Objects:  {
    Geometry: 2035541511296, "Geometry::", "Mesh" {
        Vertices: *%(vertices_num)d {
            a: %(vertices)s
        } 
        PolygonVertexIndex: *%(polygons_num)d {
            a: %(polygons)s
        } 
        GeometryVersion: 124
        %(LayerElementNormal)s
        %(LayerElementTangent)s
        %(LayerElementUV)s
        %(LayerElementColor)s
        Layer: 0 {
            Version: 100
            %(LayerElementNormalInsert)s
            %(LayerElementTangentInsert)s
            %(LayerElementUVInsert)s
            %(LayerElementColorInsert)s
        }
    }
    Model: 2035615390896, "Model::%(model_name)s", "Mesh" {
        Properties70:  {
            P: "DefaultAttributeIndex", "int", "Integer", "",0
        }
    }
}

Connections:  {
    C: "OO",2035615390896,0
    C: "OO",2035541511296,2035615390896
}
"""
        
        return dedent(FBX_TEMPLATE % ARGS).strip()

    def _generate_fbx_ascii_mixed(self, output_path, indices, all_vertex_data, mapping, unit_scale=1, coordinate_system="ue"):
        """Generate FBX ASCII file with support for mixed stage attributes
        
        Args:
            output_path: Output file path
            indices: Index list
            all_vertex_data: Dict {"vs_input": (data, attrs), "vs_output": (data, attrs)}
            mapping: Dict {semantic: (stage, attr_name)}
            unit_scale: Unit scale factor (1=cm, 100=m)
            coordinate_system: Target coordinate system ("ue", "unity", "blender", "maya")
        """
        return self._generate_fbx_ascii_with_buffer(output_path, indices, all_vertex_data, mapping, unit_scale, coordinate_system, flip_winding_order)

    def _generate_fbx_ascii_with_buffer(self, output_path, indices, all_vertex_data, mapping, unit_scale=1, coordinate_system="ue", flip_winding_order=False):
        """Generate FBX ASCII file with support for mixed stage attributes including buffer data
        
        Args:
            output_path: Output file path
            indices: Index list
            all_vertex_data: Dict {"vs_input": (data, attrs), "vs_output": (data, attrs), "buffer": (data, attrs)}
            mapping: Dict {semantic: (stage, attr_name)}
            unit_scale: Unit scale factor (1=cm, 100=m)
            coordinate_system: Target coordinate system ("ue", "unity", "blender", "maya")
            flip_winding_order: If True, flip triangle winding order (swap index 1 and 2)
        """
        from textwrap import dedent

        model_name = os.path.splitext(os.path.basename(output_path))[0]

        # Coordinate system settings for FBX
        COORD_SYSTEMS = {
            "ue": {
                "UpAxis": 2, "UpAxisSign": 1,
                "FrontAxis": 1, "FrontAxisSign": 1,
                "CoordAxis": 0, "CoordAxisSign": 1,
                "OriginalUpAxis": 2, "OriginalUpAxisSign": 1,
            },
            "unity": {
                "UpAxis": 1, "UpAxisSign": 1,
                "FrontAxis": 2, "FrontAxisSign": 1,
                "CoordAxis": 0, "CoordAxisSign": 1,
                "OriginalUpAxis": 1, "OriginalUpAxisSign": 1,
            },
            "blender": {
                "UpAxis": 2, "UpAxisSign": 1,
                "FrontAxis": 1, "FrontAxisSign": 1,
                "CoordAxis": 0, "CoordAxisSign": -1,
                "OriginalUpAxis": 2, "OriginalUpAxisSign": 1,
            },
            "maya": {
                "UpAxis": 1, "UpAxisSign": 1,
                "FrontAxis": 2, "FrontAxisSign": 1,
                "CoordAxis": 0, "CoordAxisSign": -1,
                "OriginalUpAxis": 1, "OriginalUpAxisSign": 1,
            },
        }
        
        coord = COORD_SYSTEMS.get(coordinate_system.lower(), COORD_SYSTEMS["ue"])

        def get_data(stage_attr_tuple):
            """Get vertex data from (stage, attr_name) tuple"""
            if not stage_attr_tuple:
                return None
            stage, attr_name = stage_attr_tuple
            data, attrs = all_vertex_data.get(stage, (None, []))
            if data:
                if stage == "buffer":
                    # For buffer, attr_name is the semantic (e.g., "NORMAL")
                    if attr_name in data:
                        return data[attr_name]
                elif attr_name in data:
                    return data[attr_name]
            return None

        # Prepare index data (convert to FBX polygon format)
        min_idx = min(indices) if indices else 0
        idx_list = [idx - min_idx for idx in indices]
        
        # Flip winding order if requested (swap index 1 and 2 of each triangle)
        if flip_winding_order:
            flipped_idx_list = []
            for i in range(0, len(idx_list), 3):
                if i + 2 < len(idx_list):
                    # Original: (i0, i1, i2) -> Flipped: (i0, i2, i1)
                    flipped_idx_list.extend([idx_list[i], idx_list[i+2], idx_list[i+1]])
                else:
                    flipped_idx_list.extend(idx_list[i:])
            idx_list = flipped_idx_list
            print("[export_mesh_as_fbx] Flipped winding order for %d triangles" % (len(idx_list) // 3))
        
        polygons = [str(idx ^ -1 if i % 3 == 2 else idx) for i, idx in enumerate(idx_list)]

        # Get position data
        pos_data = get_data(mapping.get('POSITION'))
        if not pos_data:
            # Fallback: search all stages for position
            for stage_name in ["vs_output", "vs_input", "buffer"]:
                data, attrs = all_vertex_data.get(stage_name, (None, []))
                if data:
                    if stage_name == "buffer":
                        for sem in attrs:
                            if 'POSITION' in sem.upper():
                                pos_data = data.get(sem)
                                if pos_data:
                                    mapping['POSITION'] = (stage_name, sem)
                                    break
                    else:
                        for attr_name in attrs:
                            if 'POS' in attr_name.upper() or 'TEXCOORD9' in attr_name.upper():
                                pos_data = data.get(attr_name)
                                if pos_data:
                                    mapping['POSITION'] = (stage_name, attr_name)
                                    break
                    if pos_data:
                        break
        
        if not pos_data:
            pos_data = []

        # Deduplicate positions for vertices section
        unique_pos = {}
        for i, idx in enumerate(idx_list):
            if idx not in unique_pos and i < len(pos_data):
                unique_pos[idx] = pos_data[i]

        vertices = [str(v) for idx in sorted(unique_pos.keys()) for v in unique_pos.get(idx, [0, 0, 0])[:3]]
        
        # Build FBX template parts
        ARGS = {
            "model_name": model_name,
            "vertices_num": len(vertices),
            "vertices": ",".join(vertices),
            "polygons_num": len(polygons),
            "polygons": ",".join(polygons),
            "unit_scale": unit_scale,
            "coordinate_system": coordinate_system.upper(),
            "UpAxis": coord["UpAxis"],
            "UpAxisSign": coord["UpAxisSign"],
            "FrontAxis": coord["FrontAxis"],
            "FrontAxisSign": coord["FrontAxisSign"],
            "CoordAxis": coord["CoordAxis"],
            "CoordAxisSign": coord["CoordAxisSign"],
            "OriginalUpAxis": coord["OriginalUpAxis"],
            "OriginalUpAxisSign": coord["OriginalUpAxisSign"],
            "LayerElementNormal": "",
            "LayerElementNormalInsert": "",
            "LayerElementTangent": "",
            "LayerElementTangentInsert": "",
            "LayerElementUV": "",
            "LayerElementUVInsert": "",
            "LayerElementColor": "",
            "LayerElementColorInsert": "",
        }
        
        # Add normals
        norm_data = get_data(mapping.get('NORMAL'))
        if norm_data:
            normals = [str(v) for values in norm_data for v in values[:3]]
            ARGS["LayerElementNormal"] = '''
                LayerElementNormal: 0 {
                    Version: 101
                    Name: ""
                    MappingInformationType: "ByPolygonVertex"
                    ReferenceInformationType: "Direct"
                    Normals: *%(normals_num)d {
                        a: %(normals)s
                    } 
                }
            ''' % {"normals": ",".join(normals), "normals_num": len(normals)}
            ARGS["LayerElementNormalInsert"] = '''
                LayerElement:  {
                    Type: "LayerElementNormal"
                    TypedIndex: 0
                }
            '''
        
        # Add UVs
        uv_data = get_data(mapping.get('UV'))
        if uv_data:
            unique_uvs = {}
            for i, idx in enumerate(idx_list):
                if idx not in unique_uvs and i < len(uv_data):
                    unique_uvs[idx] = uv_data[i]
            
            uvs = [str(1 - v if i else v) for idx in sorted(unique_uvs.keys()) 
                   for i, v in enumerate(unique_uvs.get(idx, [0, 0])[:2])]
            uv_indices = [str(idx) for idx in idx_list]
            
            ARGS["LayerElementUV"] = '''
                LayerElementUV: 0 {
                    Version: 101
                    Name: "map1"
                    MappingInformationType: "ByPolygonVertex"
                    ReferenceInformationType: "IndexToDirect"
                    UV: *%(uvs_num)d {
                        a: %(uvs)s
                    } 
                    UVIndex: *%(uv_indices_num)d {
                        a: %(uv_indices)s
                    } 
                }
            ''' % {"uvs": ",".join(uvs), "uvs_num": len(uvs), 
                   "uv_indices": ",".join(uv_indices), "uv_indices_num": len(uv_indices)}
            ARGS["LayerElementUVInsert"] = '''
                LayerElement:  {
                    Type: "LayerElementUV"
                    TypedIndex: 0
                }
            '''
        
        # Add tangents
        tan_data = get_data(mapping.get('TANGENT'))
        if tan_data:
            tangents = []
            for values in tan_data:
                if len(values) >= 4:
                    tangents.extend([str(v) for v in values[:4]])
                elif len(values) >= 3:
                    tangents.extend([str(v) for v in values[:3]] + ["1.0"])
                else:
                    tangents.extend(["1.0", "0.0", "0.0", "1.0"])
            
            ARGS["LayerElementTangent"] = '''
                LayerElementTangent: 0 {
                    Version: 101
                    Name: ""
                    MappingInformationType: "ByPolygonVertex"
                    ReferenceInformationType: "Direct"
                    Tangents: *%(tangents_num)d {
                        a: %(tangents)s
                    } 
                }
            ''' % {"tangents": ",".join(tangents), "tangents_num": len(tangents)}
            ARGS["LayerElementTangentInsert"] = '''
                LayerElement:  {
                    Type: "LayerElementTangent"
                    TypedIndex: 0
                }
            '''
        
        # Add vertex colors
        color_data = get_data(mapping.get('COLOR'))
        if color_data:
            colors = []
            for values in color_data:
                if len(values) >= 4:
                    colors.extend([str(v) for v in values[:4]])
                elif len(values) >= 3:
                    colors.extend([str(v) for v in values[:3]] + ["1.0"])
                else:
                    colors.extend(["1.0", "1.0", "1.0", "1.0"])
            
            ARGS["LayerElementColor"] = '''
                LayerElementColor: 0 {
                    Version: 101
                    Name: ""
                    MappingInformationType: "ByPolygonVertex"
                    ReferenceInformationType: "Direct"
                    Colors: *%(colors_num)d {
                        a: %(colors)s
                    } 
                }
            ''' % {"colors": ",".join(colors), "colors_num": len(colors)}
            ARGS["LayerElementColorInsert"] = '''
                LayerElement:  {
                    Type: "LayerElementColor"
                    TypedIndex: 0
                }
            '''
        
        # FBX ASCII template
        FBX_TEMPLATE = """
; FBX 7.3.0 project file
; Exported from RenderDoc MCP
; Coordinate system: %(coordinate_system)s

FBXHeaderExtension:  {
    FBXHeaderVersion: 1003
    FBXVersion: 7300
    EncryptionType: 0
    CreationTimeStamp:  {
        Version: 1000
        Year: 2024
        Month: 1
        Day: 1
        Hour: 0
        Minute: 0
        Second: 0
        Millisecond: 0
    }
    Creator: "RenderDoc MCP Exporter"
    SceneInfo: "SceneInfo::GlobalInfo", "UserData" {
        Type: "UserData"
        Version: 100
        MetaData:  {
            Version: 100
            Title: ""
            Subject: ""
            Author: ""
            Keywords: ""
            Revision: ""
            Comment: ""
        }
        Properties70:  {
            P: "DocumentUrl", "KString", "Url", "", ""
            P: "SrcDocumentUrl", "KString", "Url", "", ""
        }
    }
}
GlobalSettings:  {
    Version: 1000
    Properties70:  {
        P: "UpAxis", "int", "Integer", "",%(UpAxis)d
        P: "UpAxisSign", "int", "Integer", "",%(UpAxisSign)d
        P: "FrontAxis", "int", "Integer", "",%(FrontAxis)d
        P: "FrontAxisSign", "int", "Integer", "",%(FrontAxisSign)d
        P: "CoordAxis", "int", "Integer", "",%(CoordAxis)d
        P: "CoordAxisSign", "int", "Integer", "",%(CoordAxisSign)d
        P: "OriginalUpAxis", "int", "Integer", "",%(OriginalUpAxis)d
        P: "OriginalUpAxisSign", "int", "Integer", "",%(OriginalUpAxisSign)d
        P: "UnitScaleFactor", "double", "Number", "",%(unit_scale)s
    }
}

Documents:  {
    Count: 1
    Document: 999999999, "Scene", "Scene" {
        Properties70:  {
            P: "SourceObject", "object", "", ""
            P: "ActiveAnimStackName", "KString", "", "", ""
        }
        RootNode: 1000000000000
    }
}

References:  {
}

Definitions:  {
    Version: 100
    Count: 2
    ObjectType: "Geometry" {
        Count: 1
    }
    ObjectType: "Model" {
        Count: 1
        PropertyTemplate: "FbxNode" {
            Properties70:  {
                P: "QuaternionInterpolate", "enum", "", "",0
                P: "RotationOffset", "Vector3D", "Vector", "",0,0,0
                P: "RotationPivot", "Vector3D", "Vector", "",0,0,0
                P: "ScalingOffset", "Vector3D", "Vector", "",0,0,0
                P: "ScalingPivot", "Vector3D", "Vector", "",0,0,0
                P: "TranslationActive", "bool", "", "",0
                P: "TranslationMin", "Vector3D", "Vector", "",0,0,0
                P: "TranslationMax", "Vector3D", "Vector", "",0,0,0
                P: "TranslationMinX", "bool", "", "",0
                P: "TranslationMinY", "bool", "", "",0
                P: "TranslationMinZ", "bool", "", "",0
                P: "TranslationMaxX", "bool", "", "",0
                P: "TranslationMaxY", "bool", "", "",0
                P: "TranslationMaxZ", "bool", "", "",0
                P: "RotationOrder", "enum", "", "",0
                P: "RotationSpaceForLimitOnly", "bool", "", "",0
                P: "RotationStiffnessX", "double", "Number", "",0
                P: "RotationStiffnessY", "double", "Number", "",0
                P: "RotationStiffnessZ", "double", "Number", "",0
                P: "AxisLen", "double", "Number", "",10
                P: "PreRotation", "Vector3D", "Vector", "",0,0,0
                P: "PostRotation", "Vector3D", "Vector", "",0,0,0
                P: "RotationActive", "bool", "", "",0
                P: "RotationMin", "Vector3D", "Vector", "",0,0,0
                P: "RotationMax", "Vector3D", "Vector", "",0,0,0
                P: "RotationMinX", "bool", "", "",0
                P: "RotationMinY", "bool", "", "",0
                P: "RotationMinZ", "bool", "", "",0
                P: "RotationMaxX", "bool", "", "",0
                P: "RotationMaxY", "bool", "", "",0
                P: "RotationMaxZ", "bool", "", "",0
                P: "InheritType", "enum", "", "",0
                P: "ScalingActive", "bool", "", "",0
                P: "ScalingMin", "Vector3D", "Vector", "",0,0,0
                P: "ScalingMax", "Vector3D", "Vector", "",1,1,1
                P: "ScalingMinX", "bool", "", "",0
                P: "ScalingMinY", "bool", "", "",0
                P: "ScalingMinZ", "bool", "", "",0
                P: "ScalingMaxX", "bool", "", "",0
                P: "ScalingMaxY", "bool", "", "",0
                P: "ScalingMaxZ", "bool", "", "",0
                P: "GeometricTranslation", "Vector3D", "Vector", "",0,0,0
                P: "GeometricRotation", "Vector3D", "Vector", "",0,0,0
                P: "GeometricScaling", "Vector3D", "Vector", "",1,1,1
                P: "MinDampRangeX", "double", "Number", "",0
                P: "MinDampRangeY", "double", "Number", "",0
                P: "MinDampRangeZ", "double", "Number", "",0
                P: "MaxDampRangeX", "double", "Number", "",0
                P: "MaxDampRangeY", "double", "Number", "",0
                P: "MaxDampRangeZ", "double", "Number", "",0
                P: "MinDampStrengthX", "double", "Number", "",0
                P: "MinDampStrengthY", "double", "Number", "",0
                P: "MinDampStrengthZ", "double", "Number", "",0
                P: "MaxDampStrengthX", "double", "Number", "",0
                P: "MaxDampStrengthY", "double", "Number", "",0
                P: "MaxDampStrengthZ", "double", "Number", "",0
                P: "PreferedAngleX", "double", "Number", "",0
                P: "PreferedAngleY", "double", "Number", "",0
                P: "PreferedAngleZ", "double", "Number", "",0
                P: "LookAtProperty", "object", "", ""
                P: "UpVectorProperty", "object", "", ""
                P: "Show", "bool", "", "",1
                P: "NegativePercentShapeSupport", "bool", "", "",1
                P: "DefaultAttributeIndex", "int", "Integer", "",-1
                P: "Freeze", "bool", "", "",0
                P: "LODBox", "bool", "", "",0
                P: "Lcl Translation", "Lcl Translation", "", "A",0,0,0
                P: "Lcl Rotation", "Lcl Rotation", "", "A",0,0,0
                P: "Lcl Scaling", "Lcl Scaling", "", "A",1,1,1
                P: "Visibility", "Visibility", "", "A",1
                P: "Visibility Inheritance", "Visibility Inheritance", "", "",1
            }
        }
    }
}

Objects:  {
    Geometry: 2035541511296, "Geometry::%(model_name)s", "Mesh" {
        Properties70:  {
            P: "Color", "ColorRGB", "Color", "",0.8,0.8,0.8
        }
        Vertices: *%(vertices_num)d {
            a: %(vertices)s
        } 
        PolygonVertexIndex: *%(polygons_num)d {
            a: %(polygons)s
        } 
        GeometryVersion: 124
        %(LayerElementNormal)s
        %(LayerElementTangent)s
        %(LayerElementUV)s
        %(LayerElementColor)s
        Layer: 0 {
            Version: 100
            %(LayerElementNormalInsert)s
            %(LayerElementTangentInsert)s
            %(LayerElementUVInsert)s
            %(LayerElementColorInsert)s
        }
    }
    Model: 2035615390896, "Model::%(model_name)s", "Mesh" {
        Properties70:  {
            P: "DefaultAttributeIndex", "int", "Integer", "",0
        }
    }
}

Connections:  {
    C: "OO",2035615390896,0
    C: "OO",2035541511296,2035615390896
}
"""
        
        fbx_content = dedent(FBX_TEMPLATE % ARGS).strip()
        
        # Write to file
        output_dir = os.path.dirname(output_path)
        if output_dir:
            os.makedirs(output_dir, exist_ok=True)
        
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(fbx_content)
        
        return output_path
