# 老钱 AI 固件自动 build

这个仓库用 **GitHub Actions** 自动 build 出 **AI Passport** 的**老钱 AI 固件**（full.bin）。

## 🎯 一键流程（5 分钟）

### 第 1 步：创建 GitHub 仓库
1. 打开 https://github.com/new
2. 仓库名：`old-money-ai-passport`（任意）
3. 选 **Public**（这样 Actions 免费）
4. **不要**勾选 "Add README" / "Add .gitignore"（我们自己有）
5. 点 **Create repository**

### 第 2 步：push 代码
GitHub 创建后会显示一段 push 命令，类似：

```bash
cd ~/Desktop/old-money-ai-build
git init
git add .
git commit -m "老钱 AI 固件 build 配置"
git branch -M main
git remote add origin https://github.com/你的用户名/old-money-ai-passport.git
git push -u origin main
```

### 第 3 步：等 5-10 分钟
打开 GitHub 仓库的 **Actions** 标签 → 看 build 进度。

### 第 4 步：下载固件
Actions 完成后：
- 点最新的绿色 ✓ workflow
- 滑到 **底部 → Artifacts**
- 下载 **FoloToy-AI-Passport-full.zip**
- 解压得到 **`FoloToy-AI-Passport-full.bin`**（约 4MB）

## 🔥 烧录

拿到 `FoloToy-AI-Passport-full.bin` 后：

1. 打开 https://ai-passport.folotoy.cn/tools/web-flasher/
2. AI Passport **USB-C 连 Mac**
3. 选 .bin 文件 → **地址 0x0**
4. 选 **460800 波特率**（macOS）
5. 点 **Flash**

设备重启 → **主菜单出现 "老钱 AI"** 🎉

## 📝 包含什么

| 文件 | 内容 |
|---|---|
| `.github/workflows/build.yml` | GitHub Actions 配置 |
| `patches/main/` | 14 个老钱 AI 源码（替换官方 SDK）|
| | - menu.c/h：主菜单 |
| | - feature_quote.c/h：每日一句 |
| | - feature_diary.c/h：老钱日记 |
| | - feature_greeting.c/h：问候模式 |
| | - stock_monitor.c/h：实时盯盘 |
| | - old_money_quotes.c/h：100 条语录 |
| | - old_money_diary.c/h：30 篇日记 |
| | - CMakeLists.txt / main.c / demo.h：SDK 集成 |

## ⚙️ WiFi 配置（可选）

如果要让**实时盯盘**功能能联网拉股价，编辑 `patches/main/sdkconfig.defaults` 改：

```ini
CONFIG_STOCK_MON_WIFI_SSID="你家的WiFi名"
CONFIG_STOCK_MON_WIFI_PASS="你WiFi密码"
```

⚠️ WiFi 必须是 **2.4GHz**！

修改后重新 push → Actions 自动 rebuild → 下载新固件。

## 🆘 问题

- **Actions 显示 ❌ Failed**：点 workflow 名看日志报错，截图给我
- **build 超时**：GitHub Actions 免费版有 6 小时/任务限制，应不会
- **烧不进设备**：检查波特率是不是 460800
