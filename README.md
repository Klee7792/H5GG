# H5GG

**在 [iosgods.com](https://iosgods.com/forum/595-h5gg-igamegod/) 讨论**

一个 iOS 修改引擎，提供 JavaScript API 和 HTML5 UI。

提供类似于 Android-GG Lua API 的内存 [API](/examples-JavaScript/)。

支持从本地或网络加载脚本（*.js 或 *.html 文件）。

支持为 JavaScript API 加载 dylib 插件（[示例](/pluginDemo/customAlert)）。

支持[自动搜索值的静态指针和偏移](/examples-JavaScript/AutoSearchPointerChains.js)。

你可以使用 [HTML+CSS](/examples-HTML5/) 自定义 UI，无需电脑。

你可以通过点击一个按钮来制作自己的 tweak（dylib），非常简单！

**对于模糊搜索，建议使用：https://igg-server.herokuapp.com/**

## H5GG 支持 4 种运行模式：

1. [将 H5GG.dylib 注入到 ipa 中，用于非越狱设备](/packages/)

2. [tweak（deb）自动加载到所有应用，用于越狱设备](/packages/)

3. [独立 APP 用于越狱设备（兼容 iPad 的侧拉+分屏视图）](/appstand/packages/)

4. [屏幕悬浮窗用于越狱设备（不兼容 iPad 的侧拉+分屏视图），在 ios11~ios14 上测试](/globalview/packages/)

  还有一个 [TrollStore 的特殊版本](/appstand/packages/)


## h5gg 官方插件 [h5frida](/examples-h5frida)：

1: 支持调用任何 C/C++/Objective-C 函数（无需越狱）

2: 支持钩子任何模块的 Objective-C 方法（无需越狱）

3: 支持钩子任何模块的 C/C++ 导出函数（无需越狱）

4: 支持钩子任何模块的 C/C++ 内部函数/指令（仅限越狱）

5: **支持对应用的 C/C++ 函数/指令使用 MSHookFunction（无需越狱）**

6: **支持动态字节码补丁（patch-offset）（无需越狱）**



## 截图：
 
![text](/pictures/h5gg1.png)

![text](/pictures/h5gg2.png)

![text](/pictures/h5gg3.png)

![text](/pictures/h5gg4.PNG)



## 在 iPhone/iPad 上的 EasyHtml 中设计 Html 菜单 UI
（**从 AppStore 安装 EasyHtml！**）

![text](/pictures/easyhtml.png)



## [通过 macOS safari 调试在 iOS 上运行的 H5GG 的 js/html](https://www.lifewire.com/activate-the-debug-console-in-safari-445798)：
主机应用需要 get-task-allow 权限（越狱或通过开发者证书签名，而不是通过发布证书签名）

![text](/pictures/macos.png)


## 依赖：

悬浮 APP 的 GlobalView 模块需要这些 tweak，并且可能需要为新版本的 ios 进行更新。

+ [BackgrounderAction](https://github.com/akusio) : libH5GG.B12.dylib (jp.akusio.backgrounderaction12) 用于 ios11~ios12 

+ [BackgrounderAction2](https://github.com/akusio) : libH5GG.B.dylib (jp.akusio.backgrounderaction13) 用于 ios13+

+ [libAPAppView](https://github.com/Baw-Appie/libAPAppView) : libH5GG.A.dylib (com.rpgfarm.libapappview) 用于 ios13+





## H5GG JavaScript 引擎文档

+ [h5gg-js-doc-en.js](/docs/h5gg-js-doc-en.js) - 英文版 JavaScript API 文档
+ [h5gg-js-doc.js](/docs/h5gg-js-doc.js) - JavaScript API 文档
+ [h5gg-js-doc-deepseek.md](/docs/h5gg-js-doc-deepseek.md) - DeepSeek 文档
+ [h5gg-js-doc-doubao.md](/docs/h5gg-js-doc-doubao.md) - 豆包文档
+ [h5gg-js-doc-kimi.md](/docs/h5gg-js-doc-kimi.md) - Kimi 文档

它完全免费且开源！