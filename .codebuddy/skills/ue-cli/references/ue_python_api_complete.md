# Unreal Engine Python API 完整功能列表

## 概述

UE5 Python API 提供对编辑器功能的完整访问，主要通过以下核心库实现：

## 核心库 (Libraries)

### 1. EditorLevelLibrary - 关卡操作

**Actor 管理**
- `get_all_level_actors()` - 获取所有 Actor
- `get_selected_level_actors()` - 获取选中的 Actor
- `spawn_actor_from_class(class, location, rotation)` - 生成 Actor
- `destroy_actor(actor)` - 销毁 Actor
- `get_actor_reference(path)` - 通过路径获取 Actor

**关卡操作**
- `get_editor_world()` - 获取当前世界
- `load_level(level_path)` - 加载关卡
- `save_current_level()` - 保存当前关卡
- `new_level(level_path)` - 创建新关卡

**组件操作**
- `get_all_level_actors_components()` - 获取所有组件

### 2. EditorAssetLibrary - 资源管理

**资源查询**
- `list_assets(path, recursive=True)` - 列出资源
- `find_asset_data(asset_path)` - 查找资源数据
- `get_asset(path)` - 获取资源
- `load_asset(path)` - 加载资源
- `does_asset_exist(path)` - 检查资源是否存在

**资源操作**
- `save_asset(path)` - 保存资源
- `save_loaded_asset(asset)` - 保存已加载资源
- `delete_asset(path)` - 删除资源
- `rename_asset(old_path, new_path)` - 重命名资源
- `duplicate_asset(source, dest)` - 复制资源

**目录操作**
- `make_directory(path)` - 创建目录
- `does_directory_exist(path)` - 检查目录是否存在
- `delete_directory(path)` - 删除目录

### 3. EditorUtilityLibrary - 编辑器工具

**选择操作**
- `get_selected_assets()` - 获取选中的资源
- `get_selected_actors()` - 获取选中的 Actor
- `set_selected_actors(actors)` - 设置选中的 Actor

**UI 交互**
- `display_notification(text)` - 显示通知
- `display_dialog(title, message, type)` - 显示对话框

### 4. AssetTools - 资源工具

**资源创建**
- `create_asset(name, path, asset_class, factory)` - 创建资源
- `import_asset_tasks(tasks)` - 导入资源

**工厂类**
- `MaterialFactoryNew` - 材质工厂
- `BlueprintFactory` - 蓝图工厂
- `TextureFactory` - 纹理工厂

### 5. SystemLibrary - 系统功能

**引擎信息**
- `get_engine_version()` - 获取引擎版本
- `get_project_name()` - 获取项目名称
- `get_project_directory()` - 获取项目目录
- `get_project_content_directory()` - 获取 Content 目录

**控制台命令**
- `execute_console_command(world, command)` - 执行控制台命令

**截图**
- `take_screenshot(filename)` - 截图

### 6. MaterialEditingLibrary - 材质编辑

**材质操作**
- `create_material_expression(material, expression_class)` - 创建材质节点
- `connect_material_property(expression, output_name, property)` - 连接材质属性
- `get_material_property_input_node(material, property)` - 获取输入节点
- `update_material_instance(material)` - 更新材质实例

### 7. BlueprintEditorLibrary - 蓝图编辑

**蓝图操作**
- `spawn_actor_from_blueprint(blueprint, location, rotation)` - 从蓝图生成 Actor
- `get_blueprint_components(blueprint)` - 获取蓝图组件

### 8. EditorFilterLibrary - 过滤工具

**过滤功能**
- `by_class(objects, class_type)` - 按类过滤
- `by_name(objects, name)` - 按名称过滤

## 核心类 (Core Classes)

### Actor 类

**变换操作**
- `set_actor_location(location, sweep, teleport)` - 设置位置
- `set_actor_rotation(rotation, teleport)` - 设置旋转
- `set_actor_scale3d(scale)` - 设置缩放
- `get_actor_location()` - 获取位置
- `get_actor_rotation()` - 获取旋转
- `get_actor_scale3d()` - 获取缩放

**组件操作**
- `get_component_by_class(component_class)` - 获取组件
- `get_components_by_class(component_class)` - 获取所有组件
- `add_component(component_class, name)` - 添加组件

**属性**
- `set_actor_label(label)` - 设置显示名称
- `get_actor_label()` - 获取显示名称
- `get_name()` - 获取对象名称
- `get_class()` - 获取类

### Component 类

**StaticMeshComponent**
- `set_static_mesh(mesh)` - 设置静态网格
- `set_material(index, material)` - 设置材质
- `get_material(index)` - 获取材质

**LightComponent**
- `set_intensity(intensity)` - 设置强度
- `set_light_color(color)` - 设置颜色
- `set_attenuation_radius(radius)` - 设置衰减半径

**CameraComponent**
- `set_field_of_view(fov)` - 设置 FOV
- `set_aspect_ratio(ratio)` - 设置宽高比

### Asset 类

**Material**
- `set_editor_property(name, value)` - 设置属性
- `get_editor_property(name)` - 获取属性

**StaticMesh**
- `get_bounding_box()` - 获取边界框
- `get_socket_location(socket_name)` - 获取插槽位置

**Blueprint**
- `get_generated_class()` - 获取生成的类

## 数据结构 (Data Structures)

### 向量与旋转
- `unreal.Vector(x, y, z)` - 3D 向量
- `unreal.Rotator(pitch, yaw, roll)` - 旋转
- `unreal.Transform(location, rotation, scale)` - 变换

### 颜色
- `unreal.LinearColor(r, g, b, a)` - 线性颜色
- `unreal.Color(r, g, b, a)` - 颜色

### 其他
- `unreal.Name(string)` - 名称对象
- `unreal.Text(string)` - 文本对象

## 枚举类型 (Enums)

### MaterialProperty
- `MP_BASE_COLOR` - 基础颜色
- `MP_METALLIC` - 金属度
- `MP_SPECULAR` - 高光
- `MP_ROUGHNESS` - 粗糙度
- `MP_NORMAL` - 法线
- `MP_EMISSIVE_COLOR` - 自发光颜色

### 其他常用枚举
- `unreal.EObjectTypeQuery` - 对象类型查询
- `unreal.ECollisionChannel` - 碰撞通道

## 实用函数

### 日志
- `unreal.log(message)` - 输出日志
- `unreal.log_warning(message)` - 输出警告
- `unreal.log_error(message)` - 输出错误

### 路径
- `unreal.Paths.project_dir()` - 项目目录
- `unreal.Paths.project_content_dir()` - Content 目录
- `unreal.Paths.project_plugins_dir()` - 插件目录

### 类加载
- `unreal.load_class(package, class_name)` - 加载类
- `unreal.find_object(package, object_name)` - 查找对象

## 当前 CLI 已实现功能

### ✅ Actor 管理
- [x] 列出所有 Actor
- [x] 生成 Actor
- [x] 删除 Actor
- [x] 变换 Actor (位置/旋转/缩放)

### ✅ 关卡操作
- [x] 获取关卡信息
- [x] 打开关卡
- [x] 保存关卡

### ✅ 材质操作
- [x] 创建材质
- [x] 应用材质

### ✅ 资源管理
- [x] 列出资源
- [x] 导入资源

### ✅ 其他
- [x] 截图
- [x] 执行 Python 代码

## 可扩展功能

### 📋 建议添加

**Actor 操作**
- [ ] 获取/设置 Actor 属性
- [ ] 组件管理 (添加/删除/修改)
- [ ] Actor 标签管理
- [ ] 查找 Actor (按名称/标签/类)

**关卡操作**
- [ ] 创建新关卡
- [ ] 关卡流送控制
- [ ] 世界分区操作

**材质操作**
- [ ] 编辑材质节点
- [ ] 创建材质实例
- [ ] 设置材质参数

**蓝图操作**
- [ ] 编译蓝图
- [ ] 修改蓝图组件
- [ ] 从蓝图生成 Actor

**资源操作**
- [ ] 批量重命名
- [ ] 查找引用
- [ ] 资源迁移

**编辑器 UI**
- [ ] 显示通知
- [ ] 显示对话框
- [ ] 进度条

**选择操作**
- [ ] 获取/设置选中资源
- [ ] 获取/设置选中 Actor

**构建/烘焙**
- [ ] 构建光照
- [ ] 构建导航
- [ ] 构建反射捕获

## 参考链接

- [UE Python API 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/)
- [UE Python 脚本指南](https://docs.unrealengine.com/5.0/en-US/scripting-the-unreal-editor-using-python/)
