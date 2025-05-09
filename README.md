---
<img width="437" alt="readme" src="https://github.com/user-attachments/assets/4ea67437-d431-4931-ab74-06e0e0046f68" />


![version](https://img.shields.io/github/v/release/Juncylee/cppA25) 
![license](https://img.shields.io/badge/license-UNLICENSE-green)
# 财务助手

此项目工具读取根目录下的微信账单产生总结，通过调用 [DeepSeek 大模型 API](https://api-docs.deepseek.com/zh-cn/) 生成理财建议。

## 使用前须知

* 目前仅支持 Windows 系统，适用于其他系统的版本可能会在日后发布；  
* 目前仅支持微信账单的读取，适用于支付宝、各安卓手机厂商发行的记账程式及个性化的记录导出的账单的读取功能可能会在日后开发；  
* 使用需联网，若 AI 回复出现异常请尝试关闭代理；
* 此项目调用了 DeepSeek 大模型 API，请登录并访问 DeepSeek 开发者控制台获取 API Key；  
* 构建此项目过程中大量使用了 AI 工具以辅助，代码分发不受限制。  

## 环境准备

1.安装 MinGW-w64（[通过 MSYS2 安装](https://www.msys2.org/)）；  
2.运行 MSYS2 MINGW64，安装所需环境：  
```bash
#更新基础工具
pacman -Syu
#安装 gcc
pacman -S mingw-w64-x86_64-gcc
#安装 libcurl
pacman -S mingw-w64-x86_64-curl
#安装 nlohmann-json
pacman -S mingw-w64-x86_64-nlohmann-json
```

## 使用方法
### 一、获取账单
1.在微信内依次进入底部 **我**、**服务**、**钱包** 页面，再依次点击右上侧 **账单**、**客服中心** 按钮，选择 **下载账单** 控件；    
2.选择 **用于个人对账** 控件，按需选择账单时间，填写账单寄送至的邮箱并验证身份；  
3.检查微信支付公众号提供的解压码，从邮箱内下载并通过此解压码解压账单压缩包，得到账单 `.csv` 文件，重命名为 `data.csv` 并替换掉工具根目录内的原有的示例文件。  
<img width="334" alt="image" src="https://github.com/user-attachments/assets/b073b949-c6af-4142-9971-1d1a76998f35" />
<img width="334" alt="image" src="https://github.com/user-attachments/assets/59394fa8-650f-4457-9b2c-8119b406b656" />

**⚠️请勿泄露自己的信息给他人**。

### 二、编译（可跳过）

1.经过 AI 优化，此工具具有一定的自检能力。若不满意于目前效果，可编辑 `FinHelper.cpp` 后重新编译：  
```cpp
    json req = {
        {"model", "deepseek-reasoner"},
        {"stream", true},
        {"messages", json::array({
            {{"role","system"}, {"content","你是理财助手，请基于以下JSON数据生成总结建议。"}},
            {{"role","user"}, {"content", analysis.dump()}}
        })}
    };
```
`model` 决定了使用的模型。默认使用 `Deepseek-reasoner`（R1 模型），可选项有 `deepseek-chat`（V3模型）；  
`stream` 决定了是否以流式输出。如果设置为 true，将会以 SSE 的形式以流式发送消息增量。消息流以 data: \[DONE\] 结尾。  
请[参阅DeepSeek API 文档](https://api-docs.deepseek.com/zh-cn)。  

2.运行 MSYS2 MINGW64：
```bash
#定位至工具根目录
cd /c/文档目录/不要用反斜杠/盘符别加冒号/盘符前面还有一个斜杠/定位至文件夹即可/Finance-helper
#编译分析工具
g++ -std=c++17 DataExter.cpp -o DataExter.exe
#编译总结工具
g++ -std=c++17 FinHelper.cpp -lcurl -lssl -lcrypto -o FinHelper.exe
```
* 通过 `Ctrl` + `Incert` 组合键复制，`Shift` + `Insert` 组合键粘贴；  

### 三、运行
1.打开工具根目录内的 `apikey.txt`，填入自己的 DeepSeek API key 并保存；  
2.按住 `Shift` 在工具根目录空白处右键，选择 **在此处打开Powershell 窗口**：  
```powershell
#运行分析工具
./DataExter.exe
#运行总结工具
./Finhelper.exe
```
即可查看分析总结结果。




