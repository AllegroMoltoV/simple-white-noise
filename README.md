# シンプルホワイトノイズ / Simple White Noise

ホワイトノイズの音と、テレビの砂嵐のようなホワイトノイズの映像を出す OBS Studio のプラグインです。ソースの一覧に「ホワイトノイズ(音声)」と「ホワイトノイズ(映像)」の 2 つが加わります。音と映像は別のソースで、必要なほうを追加します。

A plugin for OBS Studio that outputs white noise as sound and as video that looks like TV static. It adds two sources to the source list, "White Noise (Audio)" and "White Noise (Video)". Sound and video are separate sources, so add the one you need.

[配信プラグイン倉庫くん](https://soko-kun.allegromoltov.jp/plugins/allegromoltov/simple-white-noise) にも掲載しています。 / Also listed on [Plugin Soko-kun](https://soko-kun.allegromoltov.jp/plugins/allegromoltov/simple-white-noise?lang=en).

## Network and data / 通信とデータ

このプラグインはネットワークへ何も送信しません。音声と映像はプラグインの中で生成し、外部のファイルやサービスを使いません。

This plugin sends nothing over the network. It generates the audio and video inside the plugin and uses no external files or services.

## Features / 機能

### ホワイトノイズ(音声) / White Noise (Audio)

- ステレオのホワイトノイズを出力します。左右のチャンネルは独立したノイズです。
- 「レベル」で、ノイズのピークのレベルを -60 dB から 0 dB の範囲で設定できます。既定値は -20 dB です。
- 音声ミキサーの音量、ミュート、モニタリングの設定は、ほかの音声ソースと同じように使えます。

- Outputs stereo white noise. The left and right channels carry independent noise.
- "Level" sets the peak level of the noise from -60 dB to 0 dB. The default is -20 dB.
- The audio mixer's volume, mute, and monitoring settings work as for any other audio source.

### ホワイトノイズ(映像) / White Noise (Video)

- 白黒の砂嵐を、フレームごとに新しく描きます。描画は GPU で行います。
- 既定ではキャンバスの大きさに合わせます。「キャンバスの大きさに合わせる」を外すと、幅と高さを指定できます。
- このソースからは音が出ません。音も必要なときは、「ホワイトノイズ(音声)」を別に追加します。

- Draws black and white static, new in every frame. The GPU does the drawing.
- By default it matches the canvas size. Turn off "Match canvas size" to set the width and height.
- This source makes no sound. To add sound, add a "White Noise (Audio)" source as well.

## Requirements / 動作環境

- Windows 10 / 11 (x64): OBS Studio 33 以降 / OBS Studio 33 or later
- Ubuntu 24.04 (x86_64): OBS Studio 31.1.1 以降 / OBS Studio 31.1.1 or later

Windows の配布物は、1.1.1 から OBS Studio 33 のプラグインの配置に合わせています。Windows で OBS Studio 32 以前を使う場合は、旧い配置の [1.1.0](https://github.com/AllegroMoltoV/simple-white-noise/releases/tag/v1.1.0) を使ってください。macOS 版は配布しません。

From 1.1.1, the Windows package follows the plugin layout of OBS Studio 33. On Windows with OBS Studio 32 or earlier, use [1.1.0](https://github.com/AllegroMoltoV/simple-white-noise/releases/tag/v1.1.0), which has the older layout. There is no macOS release.

## Installation / 導入

### Windows

1. [Releases](https://github.com/AllegroMoltoV/simple-white-noise/releases) から `simple-white-noise-<version>-windows-x64.zip` をダウンロードします。
2. OBS Studio を終了します。
3. zip の中の `simple-white-noise` フォルダーを `C:\ProgramData\obs-studio\plugins` に置きます。配置後の DLL のパスは `C:\ProgramData\obs-studio\plugins\simple-white-noise\simple-white-noise.dll` です。
4. OBS Studio を起動し、ソースの「+」から「ホワイトノイズ(音声)」または「ホワイトノイズ(映像)」を追加します。

1. Download `simple-white-noise-<version>-windows-x64.zip` from [Releases](https://github.com/AllegroMoltoV/simple-white-noise/releases).
2. Quit OBS Studio.
3. Put the `simple-white-noise` folder from the zip into `C:\ProgramData\obs-studio\plugins`. The DLL ends up at `C:\ProgramData\obs-studio\plugins\simple-white-noise\simple-white-noise.dll`.
4. Start OBS Studio and add "White Noise (Audio)" or "White Noise (Video)" with the "+" button of the Sources dock.

1.1.0 以前から更新するときも、同じ手順でフォルダーを上書きします。1.1.0 以前の DLL は `simple-white-noise\bin\64bit` にあります。OBS Studio 33 は、新しい配置の DLL を読み込めた場合、同じプラグインの旧い配置を読み込みません。更新の後は `simple-white-noise\bin` フォルダーを削除してかまいません。

To update from 1.1.0 or earlier, overwrite the folder in the same way. Those versions keep the DLL in `simple-white-noise\bin\64bit`. When OBS Studio 33 loads the DLL from the new layout, it skips the same plugin in the older layout. You can delete the `simple-white-noise\bin` folder after the update.

### Ubuntu

Ubuntu では、Releases の `simple-white-noise-<version>-x86_64-linux-gnu.deb` を `sudo apt install ./<ファイル名>` で導入します。この .deb は、OBS Studio の公式の PPA (`ppa:obsproject/obs-studio`) から apt で導入した OBS Studio を前提にしています。

On Ubuntu, install `simple-white-noise-<version>-x86_64-linux-gnu.deb` from Releases with `sudo apt install ./<file name>`. The package expects OBS Studio installed with apt from the official PPA (`ppa:obsproject/obs-studio`).

### Uninstalling / アンインストール

Windows では `C:\ProgramData\obs-studio\plugins\simple-white-noise` フォルダーを削除します。

On Windows, delete the `C:\ProgramData\obs-studio\plugins\simple-white-noise` folder.

## Verifying downloads / ダウンロードしたファイルの検証

リリースのファイルには、GitHub Actions でビルドしたことを示す build provenance の証明を付けています。[GitHub CLI](https://cli.github.com/) で確かめられます。

Every release file has a build provenance attestation made by GitHub Actions. You can verify it with the [GitHub CLI](https://cli.github.com/):

```
gh attestation verify simple-white-noise-1.1.1-windows-x64.zip --repo AllegroMoltoV/simple-white-noise
```

各ファイルの SHA-256 は、リリースのページの各ファイルの横に表示されます。

The SHA-256 of each file is shown next to it on the release page.

## Building from source / ソースからのビルド

[obs-plugintemplate](https://github.com/obsproject/obs-plugintemplate) の手順と同じです。Windows では Visual Studio 2022 で次を実行します。

The steps are the same as for [obs-plugintemplate](https://github.com/obsproject/obs-plugintemplate). On Windows, with Visual Studio 2022:

```
cmake --preset windows-x64
cmake --build build_x64 --config RelWithDebInfo
cmake --build .deps/obs-studio-31.1.1/build_x64 --config Release --target libobs-d3d11 libobs-opengl
ctest --test-dir build_x64 -C RelWithDebInfo --output-on-failure
```

3 行目は、映像のテストが使う Direct3D 11 と OpenGL の描画モジュールをビルドします。`ctest` は、libobs にプラグインを読み込んでソースを作る 2 種類のシナリオテストを実行します。

- 音声のテスト (`audio-*`) は、出力される音声のレベルと性質を確かめます。Windows では、ソースが音声を出している間のプロセスの CPU 使用率も確かめます。
- 映像のテスト (`render-*`) は、描いた画素の明るさの分布、隣の画素との相関、フレームごとの変化、大きさの設定を、Direct3D 11 と OpenGL の両方で確かめます。環境変数 `RENDER_TEST_OUTPUT_DIR` にフォルダーを指定すると、描いた画像を BMP で保存します。

CI では、音声のテストを Windows と Ubuntu で、映像のテストを Windows の Direct3D 11 と Ubuntu の OpenGL で実行します。CPU 使用率の確認は Windows だけで行います。

The third line builds the Direct3D 11 and OpenGL graphics modules that the video tests use. `ctest` runs two kinds of scenario tests that load the plugin into libobs and create the sources.

- The audio tests (`audio-*`) check the level and properties of the audio the source outputs. On Windows, they also check the CPU usage of the process while the source outputs audio.
- The video tests (`render-*`) check the distribution of pixel levels, the correlation with neighboring pixels, the change between frames, and the size settings, with both Direct3D 11 and OpenGL. Set the environment variable `RENDER_TEST_OUTPUT_DIR` to a folder to save the rendered images as BMP files.

CI runs the audio tests on Windows and Ubuntu, and the video tests with Direct3D 11 on Windows and OpenGL on Ubuntu. It checks the CPU usage on Windows only.

## Use of AI / 生成 AI の利用

このプラグインのコードと文書は、Claude Code (Anthropic) を使って作成しました。

The code and documents of this plugin were written with Claude Code (Anthropic).

## License / ライセンス

GPL-2.0-or-later。詳細は [LICENSE](LICENSE) を参照してください。

GPL-2.0-or-later. See [LICENSE](LICENSE).

## Changelog / 更新情報

### 1.1.1

- Windows の配布物を OBS Studio 33 のプラグインの配置に合わせました。DLL はプラグインのフォルダーの直下に入ります。Windows で OBS Studio 32 以前を使う場合は 1.1.0 を使ってください / The Windows package now follows the plugin layout of OBS Studio 33, with the DLL directly in the plugin folder. On Windows with OBS Studio 32 or earlier, use 1.1.0

### 1.1.0

- テレビの砂嵐のような映像を出す「ホワイトノイズ(映像)」のソースを追加しました / Added the "White Noise (Video)" source, which outputs video that looks like TV static
- 音声のソースの表示名を「ホワイトノイズ(音声)」に変えました。既存のシーンのソースはそのまま使えます / Renamed the audio source to "White Noise (Audio)". Sources in existing scenes keep working

### 1.0.1

- Windows で、ノイズを生成するスレッドが待つ間に CPU を空回りで使わないようにしました / On Windows, the thread that generates the noise no longer busy-waits on the CPU between chunks

### 1.0.0

- 最初の公開版 / First public release
