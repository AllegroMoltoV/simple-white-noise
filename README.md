# シンプルホワイトノイズ / Simple White Noise

ホワイトノイズを鳴らす OBS Studio の音声ソースのプラグインです。ソースの一覧から「ホワイトノイズ」を追加し、プロパティでノイズのレベルを調整できます。

An audio source plugin for OBS Studio that plays white noise. Add "White Noise" from the source list and adjust its level in the properties.

## 通信とデータ / Network and data

このプラグインはネットワークへ何も送信しません。音声はプラグインの中で生成し、外部のファイルやサービスを使いません。

This plugin sends nothing over the network. It generates the audio inside the plugin and uses no external files or services.

## 機能 / Features

- ステレオのホワイトノイズを出力します。左右のチャンネルは独立したノイズです。
- 「レベル」で、ノイズのピークのレベルを -60 dB から 0 dB の範囲で設定できます。既定値は -20 dB です。
- 音声ミキサーの音量、ミュート、モニタリングの設定は、ほかの音声ソースと同じように使えます。

- Outputs stereo white noise. The left and right channels carry independent noise.
- "Level" sets the peak level of the noise from -60 dB to 0 dB. The default is -20 dB.
- The audio mixer's volume, mute, and monitoring settings work as for any other audio source.

## 動作環境 / Requirements

- OBS Studio 31.1.1 以降 / OBS Studio 31.1.1 or later
- Windows 10 / 11 (x64)、Ubuntu 24.04 (x86_64)

macOS 版は配布しません。

There is no macOS release.

## 導入 / Installation

### Windows

1. [Releases](https://github.com/AllegroMoltoV/simple-white-noise/releases) から `simple-white-noise-<version>-windows-x64.zip` をダウンロードします。
2. OBS Studio を終了します。
3. zip の中の `simple-white-noise` フォルダーを `C:\ProgramData\obs-studio\plugins` に置きます。配置後の DLL のパスは `C:\ProgramData\obs-studio\plugins\simple-white-noise\bin\64bit\simple-white-noise.dll` です。
4. OBS Studio を起動し、ソースの「+」から「ホワイトノイズ」を追加します。

1. Download `simple-white-noise-<version>-windows-x64.zip` from [Releases](https://github.com/AllegroMoltoV/simple-white-noise/releases).
2. Quit OBS Studio.
3. Put the `simple-white-noise` folder from the zip into `C:\ProgramData\obs-studio\plugins`. The DLL ends up at `C:\ProgramData\obs-studio\plugins\simple-white-noise\bin\64bit\simple-white-noise.dll`.
4. Start OBS Studio and add "White Noise" with the "+" button of the Sources dock.

### Ubuntu

Releases の `simple-white-noise-<version>-x86_64-linux-gnu.deb` を、パッケージマネージャーで導入します。

Install `simple-white-noise-<version>-x86_64-linux-gnu.deb` from Releases with your package manager.

### アンインストール / Uninstalling

Windows では `C:\ProgramData\obs-studio\plugins\simple-white-noise` フォルダーを削除します。

On Windows, delete the `C:\ProgramData\obs-studio\plugins\simple-white-noise` folder.

## ダウンロードしたファイルの検証 / Verifying downloads

リリースのファイルには、GitHub Actions でビルドしたことを示す build provenance の証明を付けています。[GitHub CLI](https://cli.github.com/) で確かめられます。

Every release file has a build provenance attestation made by GitHub Actions. You can verify it with the [GitHub CLI](https://cli.github.com/):

```
gh attestation verify simple-white-noise-1.0.0-windows-x64.zip --repo AllegroMoltoV/simple-white-noise
```

各ファイルの SHA-256 は、リリースのページの各ファイルの横に表示されます。

The SHA-256 of each file is shown next to it on the release page.

## ソースからのビルド / Building from source

[obs-plugintemplate](https://github.com/obsproject/obs-plugintemplate) の手順と同じです。Windows では Visual Studio 2022 で次を実行します。

The steps are the same as for [obs-plugintemplate](https://github.com/obsproject/obs-plugintemplate). On Windows, with Visual Studio 2022:

```
cmake --preset windows-x64
cmake --build build_x64 --config RelWithDebInfo
ctest --test-dir build_x64 -C RelWithDebInfo --output-on-failure
```

`ctest` は、libobs にプラグインを読み込んでソースを作り、出力される音声のレベルと性質を確かめるシナリオテストを実行します。CI では同じテストを Windows と Ubuntu で実行します。

`ctest` runs a scenario test that loads the plugin into libobs, creates the source, and checks the level and properties of the audio it outputs. CI runs the same test on Windows and Ubuntu.

## 生成 AI の利用 / Use of AI

このプラグインのコードと文書は、Claude Code (Anthropic) を使って作成しました。

The code and documents of this plugin were written with Claude Code (Anthropic).

## ライセンス / License

GPL-2.0-or-later。詳細は [LICENSE](LICENSE) を参照してください。

GPL-2.0-or-later. See [LICENSE](LICENSE).

## 更新情報 / Changelog

### 1.0.0

- 最初の公開版 / First public release
