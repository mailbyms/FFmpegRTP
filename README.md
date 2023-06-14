## 一、项目来源
- 原始项目代码来自 stackoverflow：https://stackoverflow.com/questions/46352604/writing-image-to-rtp-with-ffmpeg    

## 二、项目建立
Visual Studio 2017， Windows 10， 正常运行
- 下载 FFMPEG 64位 SDK (https://www.gyan.dev/ffmpeg/builds/packages/ffmpeg-4.3.2-full_build-shared.7z)，并解压到 C 盘根目录下

- IDE 添加 FFMPEG SDK:
	
	- 项目属性，切换到 Debug - x64
	  ```
	  C/C++
	      常规->附加目录：增加 ffmpeg 的 include 目录，例如"C:\ffmpeg-4.3.2\include"
	      常规->SDL检查：改为“否”
	  链接器
	      常规->附加库目录：增加 ffmpeg 的 lib 目录，例如"C:\ffmpeg-4.3.2\lib"
	  调试
	      环境：PATH=%PATH%;C:\ffmpeg-4.3.2\bin
	  ```

## 三、运行

启动后，控制台会打印 sdp 内容，程序暂停（本地会生成 test.sdp）  

按任意键后，会启动电脑上的 vlc 程序播放  

也可以关掉 VLC，使用 ffplay 播放 `ffplay -protocol_whitelist file,udp,rtp test.sdp`



