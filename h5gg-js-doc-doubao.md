# H5GG JS脚本引擎文档（v7.5更新）
## 一、数值类型说明
H5GG支持的数值类型分为以下三类，所有类型参数均需传入字符串格式：

| 类型分类 | 具体类型 | 说明 |
|----------|----------|------|
| 浮点类型 | F32、F64 | 32位/64位浮点数 |
| 有符号数 | I8、I16、I32、I64 | 8/16/32/64位有符号整数 |
| 无符号数 | U8、U16、U32、U64 | 8/16/32/64位无符号整数 |

## 二、核心API说明及示例
### 2.1 基础配置类
#### h5gg.require(版本号)
- **作用**：设定脚本需要的最低H5GG版本号，建议写在脚本开头第一行
- **参数**：版本号（字符串格式，如"7.5"）
- **示例**：
  ```javascript
  h5gg.require("7.5"); // 要求最低版本为7.5
  ```

#### h5gg.setFloatTolerance(偏差值)
- **作用**：设置F32/F64浮点搜索的偏差范围，引擎默认0.0
- **参数**：浮点偏差值（字符串格式，如"0.1"）
- **示例**：
  ```javascript
  h5gg.setFloatTolerance("0.01"); // 浮点搜索偏差设为0.01
  ```

### 2.2 内存搜索类
#### h5gg.searchNumber(数值, 类型, 搜索下限, 搜索上限)
- **作用**：搜索/二次搜索指定范围、指定类型的精确数值
- **参数**：
  - 数值：支持单个值（如"10086"）或范围（如"50~100"，"2.3~7.8"），字符串格式
  - 类型：数值类型（如"F32"），字符串格式
  - 搜索下限/上限：地址范围，支持十进制或0x开头十六进制，字符串格式
- **示例**：
  ```javascript
  // 搜索0x100000000~0x1600000000范围内F32类型值为10086的数据
  h5gg.searchNumber("10086", "F32", "0x100000000", "0x1600000000");
  
  // 搜索50~100范围内I32类型的数据
  h5gg.searchNumber("50~100", "I32", "0", "0xFFFFFFFF");
  ```

#### h5gg.searchNearby(数值, 类型, 邻近范围)
- **作用**：邻近（联合）搜索，与IGG逻辑一致
- **参数**：
  - 数值：支持单个值/范围，字符串格式
  - 类型：数值类型，字符串格式
  - 邻近范围：地址范围（如"0x100"），字符串格式
- **示例**：
  ```javascript
  // 先精确搜索，再联合搜索邻近0x100范围的F32类型值为1008611的数据
  h5gg.searchNumber("10086", "F32", "0x100000000", "0x1600000000");
  h5gg.searchNearby("1008611", "F32", "0x100");
  ```

### 2.3 内存读写/修改类
#### h5gg.getValue(地址, 类型)
- **作用**：读取指定地址的数值，返回数值字符串
- **参数**：
  - 地址：十进制/0x开头十六进制，字符串格式
  - 类型：数值类型，字符串格式
- **示例**：
  ```javascript
  // 读取0x1A2B3CD地址的I32类型数值
  var value = h5gg.getValue("0x1A2B3CD", "I32");
  // 转换为数字类型后运算
  var numValue = Number(value) + 10;
  console.log("运算后值：" + numValue);
  ```

#### h5gg.setValue(地址, 数值, 类型)
- **作用**：设置指定地址的数值，返回成功/失败
- **参数**：
  - 地址：十进制/0x开头十六进制，字符串格式
  - 数值：要设置的值，字符串格式
  - 类型：数值类型，字符串格式
- **示例**：
  ```javascript
  // 将0x1A2B3CD地址的I32类型值设为999
  var result = h5gg.setValue("0x1A2B3CD", "999", "I32");
  console.log("修改结果：" + (result ? "成功" : "失败"));
  
  // 十进制地址示例：将12345678地址的U32类型值设为100
  h5gg.setValue("12345678", "100", "U32");
  ```

#### h5gg.editAll(数值, 类型)
- **作用**：修改当前搜索结果中的所有数值（清除结果后不可调用），返回修改成功个数
- **参数**：
  - 数值：要设置的值，字符串格式
  - 类型：数值类型，字符串格式
- **示例**：
  ```javascript
  // 搜索并批量修改F32类型值为10086的数据为10010
  h5gg.searchNumber("10086", "F32", "0x100000000", "0x1600000000");
  var editCount = h5gg.editAll("10010", "F32");
  console.log("修改成功个数：" + editCount);
  h5gg.clearResults();
  ```

### 2.4 搜索结果管理类
#### h5gg.getResultsCount()
- **作用**：获取搜索结果的总数量，返回数字类型
- **示例**：
  ```javascript
  h5gg.searchNumber("10086", "F32", "0x100000000", "0x1600000000");
  var count = h5gg.getResultsCount();
  console.log("搜索结果总数：" + count);
  ```

#### h5gg.getResults(获取个数, 跳过个数)
- **作用**：分段获取搜索结果数组，每个元素包含`address`（地址）、`value`（数值）、`type`（类型）属性
- **参数**：获取个数、跳过个数（均为字符串格式）
- **注意**：避免一次性获取全部数据，防止内存溢出闪退
- **示例**：
  ```javascript
  h5gg.searchNumber("10086", "F32", "0x100000000", "0x1600000000");
  var total = h5gg.getResultsCount();
  var batchSize = 100; // 每次获取100条
  // 分段遍历所有结果
  for (var i = 0; i < total; i += batchSize) {
    var results = h5gg.getResults(String(batchSize), String(i));
    for (var j = 0; j < results.length; j++) {
      var addr = results[j].address;
      var val = results[j].value;
      console.log("地址：" + addr + "，值：" + val);
    }
  }
  ```

#### h5gg.clearResults()
- **作用**：清除搜索结果，用于开始新的搜索
- **示例**：
  ```javascript
  h5gg.searchNumber("10086", "F32", "0x100000000", "0x1600000000");
  h5gg.clearResults(); // 清空当前搜索结果
  ```

### 2.5 模块/基址操作类
#### h5gg.getRangesList(模块文件名)
- **作用**：返回模块数组，每个模块包含`start`（基址）、`end`（结尾地址）、`name`（路径）属性
- **参数**：
  - 模块文件名：字符串格式；传"0"返回APP主程序模块；不传返回所有模块
- **示例**：
  ```javascript
  // 获取APP主程序模块信息
  var mainModule = h5gg.getRangesList("0")[0];
  var baseAddr = mainModule.start;
  console.log("主程序基址：" + baseAddr);
  
  // 获取所有模块列表
  var allModules = h5gg.getRangesList();
  allModules.forEach(function(module, index) {
    console.log("模块" + index + "：" + module.name + "，范围：" + module.start + "~" + module.end);
  });
  ```

#### fuckbase(地址, 字节)
- **作用**：修改静态基址
- **参数**：
  - 地址：十进制/0x开头十六进制，字符串格式
  - 字节：要写入的字节串，字符串格式
- **示例**：
  ```javascript
  // 获取ShadowTrackerExtra模块基址并偏移修改
  var module = h5gg.getRangesList("ShadowTrackerExtra")[0];
  var baseAddr = Number(module.start) + 0x02EF7144; // 计算目标地址
  fuckbase(String(baseAddr), "00F0271E0008201EC0035FD6"); // 修改基址字节
  ```

### 2.6 插件加载类
#### h5gg.loadPlugin(OC类名, dylib路径)
- **作用**：加载dylib插件，返回OC类名的对象实例（可直接调用OC方法）
- **参数**：
  - OC类名：字符串格式
  - dylib路径：绝对路径/.app内相对路径，字符串格式
- **示例**：
  ```javascript
  // 加载插件并调用OC方法
  var plugin = h5gg.loadPlugin("MyOCClass", "/var/containers/Bundle/Application/XXX/XXX.app/MyPlugin.dylib");
  if (plugin) {
    plugin.myOCMethod("参数1", "参数2"); // 调用OC类的方法
  }
  ```

### 2.7 跨进程操作类（跨进程版专用）
#### h5gg.setTargetProc(进程号)
- **作用**：设定当前目标进程，返回成功/失败
- **参数**：进程号（字符串格式）
- **示例**：
  ```javascript
  // 先获取进程列表，再设置目标进程
  var procs = h5gg.getProcList("ShadowTrackerExtra");
  if (procs.length > 0) {
    var pid = procs[0].pid;
    var result = h5gg.setTargetProc(pid);
    console.log("设置目标进程" + pid + "：" + (result ? "成功" : "失败"));
  }
  ```

#### h5gg.getProcList(进程名)
- **作用**：获取进程数组，每个元素包含`pid`（进程号）、`name`（进程名）属性
- **参数**：进程名（字符串格式；不传返回所有运行中的APP进程）
- **示例**：
  ```javascript
  // 获取所有运行中的APP进程
  var allProcs = h5gg.getProcList();
  allProcs.forEach(function(proc) {
    console.log("进程名：" + proc.name + "，进程号：" + proc.pid);
  });
  
  // 定时检测目标进程是否存活
  setInterval(function() {
    var procs = h5gg.getProcList("ShadowTrackerExtra");
    if (procs.length === 0) {
      console.log("目标进程已退出");
    }
  }, 5000);
  ```

### 2.8 悬浮窗/界面控制类
#### setButtonImage(图标)
- **作用**：设置悬浮按钮图标
- **参数**：图标（http开头的网络图片URL/Base64编码的DataURL）
- **示例**：
  ```javascript
  // 网络图片设置图标
  setButtonImage("http://example.com/icon.png");
  
  // Base64图片设置图标（示例，需替换为真实Base64）
  setButtonImage("data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAA...");
  ```

#### setButtonAction(回调函数)
- **作用**：设置悬浮按钮点击事件的回调函数
- **示例**：
  ```javascript
  // 点击悬浮按钮时执行的逻辑
  setButtonAction(function() {
    alert("悬浮按钮被点击！");
    // 示例：点击后修改指定地址数值
    h5gg.setValue("0x1A2B3CD", "999", "I32");
  });
  ```

#### setWindowRect(x, y, 宽度, 高度)
- **作用**：修改悬浮窗口在屏幕中的位置和尺寸
- **参数**：x/y坐标、宽度/高度（均为数字类型）
- **示例**：
  ```javascript
  // 设置悬浮窗位置为(100, 200)，尺寸为400x400
  setWindowRect(100, 200, 400, 400);
  ```

#### setWindowDrag(x, y, 宽度, 高度)
- **作用**：设置悬浮窗口中可拖动的区域
- **参数**：拖动区域的x/y坐标、宽度/高度（数字类型）
- **示例**：
  ```javascript
  // 设置悬浮窗左上角100x100区域可拖动
  setWindowDrag(0, 0, 100, 100);
  ```

#### setWindowTouch(是否响应触控)
- **作用**：设置悬浮窗触摸穿透性
- **参数**：true（不可穿透）/false（可穿透）
- **示例**：
  ```javascript
  setWindowTouch(true); // 悬浮窗触摸不可穿透
  ```

#### setWindowVisible(显示状态)
- **作用**：设置悬浮窗可见性
- **参数**：true（显示）/false（隐藏）
- **示例**：
  ```javascript
  // 隐藏悬浮窗
  setWindowVisible(false);
  // 5秒后显示悬浮窗
  setTimeout(function() {
    setWindowVisible(true);
  }, 5000);
  ```

#### setLayoutAction(回调函数)
- **作用**：设置屏幕旋转/iPad分屏变化时的回调函数（参数为宽、高）
- **示例**：
  ```javascript
  // 屏幕尺寸变化时调整悬浮窗大小
  setLayoutAction(function(width, height) {
    console.log("屏幕尺寸变化：" + width + "x" + height);
    // 适配屏幕宽度，设置悬浮窗宽度为屏幕1/2
    setWindowRect(100, 200, width/2, 400);
  });
  ```

## 三、高级示例
### 3.1 地址偏移修改（匹配指定结尾地址）
```javascript
// 搜索I32类型值为1078355558的数据，修改以8D4结尾地址及偏移-48的地址值
h5gg.searchNumber("1078355558", "I32", "0x100000000", "0x1600000000");
var count = h5gg.getResultsCount();
var results = h5gg.getResults(String(count), "0"); // 获取所有结果（小数据量场景）

for (var i = 0; i < count; i++) {
  var addr1 = results[i].address;
  var regex = /8D4$/; // 匹配以8D4结尾的地址（H5GG地址反写）
  if (regex.test(addr1)) {
    // 转换地址为数字类型进行偏移计算
    var addr2Num = Number(addr1);
    var addr3Num = addr2Num - 48; // 向上偏移48（每偏移1位±4）
    
    // 修改原地址值为18，偏移地址值为0
    h5gg.setValue(String(addr2Num), "18", "I32");
    h5gg.setValue(String(addr3Num), "0", "I32");
    
    console.log("修改地址：" + addr2Num + "，偏移地址：" + addr3Num);
  }
}
h5gg.clearResults();
```

### 3.2 循环定时修改指定地址
```javascript
// 单线程友好的定时循环修改（避免卡顿）
var targetAddr = "0x1A2B3CD"; // 目标地址
// 每1毫秒修改一次值为999（I32类型）
var intervalId = setInterval(function() {
  var result = h5gg.setValue(targetAddr, "999", "I32");
  if (!result) {
    console.log("修改失败，停止循环");
    clearInterval(intervalId); // 修改失败时停止循环
  }
}, 1);

// 10秒后停止循环
setTimeout(function() {
  clearInterval(intervalId);
  console.log("定时修改停止");
}, 10000);
```

### 3.3 范围搜索+联合搜索精准定位
```javascript
// 多轮搜索缩小范围：先范围搜索，再联合搜索，最后精确修改
h5gg.require("7.5");
h5gg.setFloatTolerance("0.01"); // 浮点偏差设为0.01

// 第一步：范围搜索20~30之间的F64类型数据
h5gg.searchNumber("20~30", "F64", "0x100000000", "0x1600000000");
var firstCount = h5gg.getResultsCount();
console.log("第一轮范围搜索结果：" + firstCount + "条");

// 第二步：联合搜索邻近0x200范围、值为25.5~26.5的F64类型数据
h5gg.searchNearby("25.5~26.5", "F64", "0x200");
var secondCount = h5gg.getResultsCount();
console.log("第二轮联合搜索结果：" + secondCount + "条");

// 第三步：精确搜索值为26的F64类型数据
h5gg.searchNumber("26", "F64", "0x100000000", "0x1600000000");
var finalCount = h5gg.getResultsCount();
console.log("最终精确搜索结果：" + finalCount + "条");

// 批量修改为30
if (finalCount > 0) {
  var editCount = h5gg.editAll("30", "F64");
  console.log("修改成功：" + editCount + "条");
}

h5gg.clearResults();
```

## 四、注意事项
1. **参数格式**：地址支持十进制/0x开头十六进制（自动识别），其余参数必须为字符串格式；
2. **数据类型转换**：搜索结果的地址/数值为字符串类型，运算前需用`Number(x)`转换为数字类型；
3. **数组索引**：JS数组索引从0开始（区别于Lua的1开始）；
4. **十六进制转换**：数字类型可通过`x.toString(16)`转换为十六进制字符串；
5. **内存优化**：搜索结果量大时，需分段调用`h5gg.getResults`，避免内存溢出；
6. **JS运算注意**：`+`号在JS中若连接字符串会直接拼接，需确保运算数为数字类型；
7. **悬浮窗默认值**：默认尺寸370x370，可在H5启动时通过`setWindowRect`调整；
8. **浮点搜索**：可通过`h5gg.setFloatTolerance`设置偏差，适配浮点数值的精度问题。