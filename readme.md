### 基于windows下的音频采集与播放

> 本项目主要实现windows平台下音频的采集与播放， 使用了微软官方`AduioCoreAPI` 开发。 同时还实现了音频编解码(基于faac/faad2) ,
> 
> 可以直接采集音频录制为`aac`文件， 也可以直接播放`aac`文件



#### 主要API

+ `IAudioSink LoadData`

+ `IAudioData CopyData`



