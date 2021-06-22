# lvgl_clock

一个时钟

# 使用方法

```bash
git clone https://github.com/Noooooob233/lvgl_clock
cd lvgl_clock
make
```

第一次请先运行一次生成.config配置文件, 并在配置文件中修改你的位置以及心知天气的秘钥

```bash
{
	"weather":	{
		"location":	"beijing",
		"key":	"YOR_PRIVATE_KEY"
	}
}
```

# 添加开机自启

```bash
nano /usr/lib/systemd/system/lvgl_clock.service
```

添加以下内容

```bash
[Unit]
After=network.target

[Service]
ExecStart=/YOUR_PATH/lvgl_clock
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

添加完成后开启自启

```bash
systemctl enable lvgl_clock.service
systemctl start lvgl_clock.service
```
