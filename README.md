# H5GG

[![English Readme](https://img.shields.io/badge/Lang-English-blue)](./README.en-US.md)
![Support](https://img.shields.io/badge/Support-TrollStore-blue)

> **Tips:** ：Click the「Lang English」badge above to view the English documentation.

> **在 [![Discord](https://img.shields.io/badge/Discord-5865F2?logo=discord&logoColor=white)](https://discord.gg/FAs4MH7HMc) 或 [![iOSGods](https://img.shields.io/badge/iOSGods-red)](https://iosgods.com/forum/595-h5gg-igamegod/) 中讨论**

一款带有JavaScript API和Html5用户界面的iOS修改引擎。

提供类似Android版GG修改器的Lua API的[内存API](/examples-JavaScript/)。

支持从本地或网络加载脚本（*.js或*.html文件）。

支持加载dylib插件以扩展JavaScript API（[示例](/pluginDemo/customAlert)）。

支持[自动搜索数值的静态指针和偏移量](/examples-JavaScript/AutoSearchPointerChains.js)。

你可以使用[HTML+CSS](/examples-HTML5/)在没有电脑的情况下自定义界面。

你还可以一键制作属于自己的插件（dylib），非常简单！

**对于模糊搜索，建议使用：https://igg-server.herokuapp.com/**

## H5GG支持4种运行模式：

1. [向IPA注入H5GG.dylib，适用于非越狱设备](/packages/)

2. [插件（deb）自动加载到所有应用，适用于越狱设备](/packages/)

3. [独立应用，适用于越狱设备（兼容iPad的侧拉+分屏功能）](/appstand/packages/)

4. [悬浮窗模式，适用于越狱设备（不兼容iPad的侧拉+分屏功能），已在iOS11~iOS14上测试](/globalview/packages/)

   还有[专为TrollStore设计的特殊版本](/appstand/packages/)


## h5gg-official-plugin [h5frida](/examples-h5frida)：

1：支持调用任意C/C++/Objective-C函数（无需越狱）

2：支持Hook任意模块的Objective-C方法（无需越狱）

3：支持Hook任意模块的C/C++导出函数（无需越狱）

4：支持Hook任意模块的C/C++内部函数/指令（仅越狱设备）

5：**支持对应用的C/C++函数/指令进行MSHookFunction（无需越狱）**

6：**支持动态字节码补丁（patch-offset）（无需越狱）**



## 截图：
 
![text](/pictures/h5gg1.png)

![text](/pictures/h5gg2.png)

![text](/pictures/h5gg3.png)

![text](/pictures/h5gg4.PNG)



## 在iPhone/iPad的EasyHtml中设计Html菜单界面
**（从AppStore安装EasyHtml！）**

![text](/pictures/easyhtml.png)



## [通过macOS Safari调试运行在iOS上的H5GG的js/html](https://www.lifewire.com/activate-the-debug-console-in-safari-445798)：
宿主应用需要具备get-task-allow权限（需越狱或使用开发者证书签名，不能使用分发证书签名）

![text](/pictures/macos.png)


## 依赖项：

悬浮窗应用的全局视图模块需要以下插件，并可能需要针对新iOS版本进行更新。

+ [BackgrounderAction](https://github.com/akusio) ：libH5GG.B12.dylib（jp.akusio.backgrounderaction12）适用于iOS11~iOS12 

+ [BackgrounderAction2](https://github.com/akusio) ：libH5GG.B.dylib（jp.akusio.backgrounderaction13）适用于iOS13+

+ [libAPAppView](https://github.com/Baw-Appie/libAPAppView) ：libH5GG.A.dylib（com.rpgfarm.libapappview）适用于iOS13+


## H5GG JavaScript引擎文档

+ [H5GG JavaScript引擎文档](/h5gg-js-doc.md)
+ [H5GG JavaScript引擎文档(DeepSeek)](/h5gg-js-doc-deepseek.md)
+ [H5GG JavaScript引擎文档(Kimi)](/h5gg-js-doc-kimi.md)
+ [H5GG JavaScript引擎文档(豆包)](/h5gg-js-doc-doubao.md)


完全免费且开源！
