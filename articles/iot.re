= IoTを活用したモニタリング

収集して得たさまざまな値は多くの知見を与えてくれます。
（理想はセンサーの値によって給水をしたり温度を調整するといったフローの自動化ができれば良いのですが、今後の課題です）

この章ではラズベリーパイと環境センサーを用いた値の取得方法について解説します。

ただデータが取れるだけでは面白くないので、それを外部に飛ばして可視化する方法についても触れます。

比較的簡単な手順でかつ無料に使用できるThingSpeakというサービスを使っています。

筆者はThingSpeakで得たデータをGrafanaでデータを参照し、ダッシュボードを作っているので、それについても合わせて触れます。

== 用意するもの

 * ラズベリーパイ（4B）@<fn>{raspberry_pi}
 * 環境センサ
 ** Omronから発売されている 2JCIE-BU01(F1) という機種です
 ** これをラズパイにUSBに接続します。ちなみに公式アプリをアプリストアからダウンロードすれば、特に何の設定もすることなく、スマホからデータの確認が可能です。@<fn>{end_of_service}

//image[omron_app1][アプリ画面その１][scale=0.5]
//image[omron_app2][アプリ画面その２][scale=0.5]
//footnote[raspberry_pi][特に詳しくは触れません。前はもっと安かった気がしますが、今一万円以上するみたいです。]
//footnote[end_of_service][しかし2025年9月でアプリのサポートが終了するようです。機器本体も2025年1月で生産終了予定...]

== 環境センサを使用するための準備
まず、環境センサをラズパイにUSBで繋ぎます。（bluetoothでの接続は筆者の環境ではうまくいかなかったのでUSBで接続してデータを取得する方法を取っています）

=== 環境センサを使用するにあたっての操作

こちらの記事を参考にしています。
@<href>{https://armadillo.atmark-techno.com/howto/armadillo_2JCIEBU01_USBcom, 「オムロン 環境センサ(USB)」からUSB通信を用いたデータ収集}

USBのポート番号だけ調べて、特に設定はせずに使えました。

以上で事前準備は完了です。

== サンプルコードの紹介
筆者が作成して使用している、ThingSpeakへのデータ送信のためのサンプルリポジトリを紹介します。

ThingSpeakというサービスを聞き慣れない方に簡単に説明すると、IoTデータを可視化するためのサービスです。@<fn>{thingspeak}

//footnote[thingspeak][本当はGoogleスプレッドシートへのデータ送信を行なっていたのですが、アカウント作成から手順化するのが面倒だったので、方法を変えました。しかし、この方が安定してGrafanaからのデータ参照ができるし、コードもシンプルになったので結果オーライです]

=== 実行前の準備

まず、GitHubから@<href>{https://github.com/Mutsumix/RasPi-EnvSensor-ThingSpeak, https://github.com/Mutsumix/RasPi-EnvSensor-ThingSpeak}のリポジトリをクローンします。

続いて、必要なライブラリをインストールします。

//cmd{
$ pip install -r requirements.txt
//}

設定ファイルをコピーして編集します。

//cmd{
$ cp config.yml.example config.yml
$ nano config.yml
//}

 * データの取得頻度（分単位）
 * ThingSpeakのAPIキー
 * 環境センサのポート番号

=== 実行

プログラムの実行

//cmd{
$ python main.py
//}

値がThingSpeakに書き込まれていることを確認します。


=== Grafanaでダッシュボードを作成する
これで終了でも良いのですが、あまり面白くないので、私はGrafabaにダッシュボードを作成しています。
こうすれば、視覚的に今どうなっているのか把握しやすいですし、外出先でも簡単に生育状況を確認することができます。

1. Grafana Cloudアカウントの作成:

Grafana Cloudのウェブサイト（@<href>{https://grafana.com/products/cloud/, Grafana Cloud}）にアクセスします。

"Create free account"をクリックし、指示に従ってアカウントを作成します。

アカウント作成後、Grafana Cloudのダッシュボードにログインします。

2. Grafana Cloudインスタンスへのアクセス:

Grafana Cloudダッシュボードから、あなたのGrafanaインスタンスにアクセスします。

初回ログイン時にパスワードの変更を求められる場合があります。

3. ThingSpeakデータソースの設定:

左側のメニューから「Connection」> 「add new connection」を選択します。

「Add data source」をクリックします。

検索バーで "JSON" と検索し、「JSON API」を選択します。

installします

左側のメニューから「Connection」> 「Data sources」を選択します。

以下の設定を行います：
 * Name: ThingSpeak
 * URL: https://api.thingspeak.com/channels/CHANNEL_ID/feeds.json?api_key=YOUR_API_KEY の形式です（CHANNEL_ID と YOUR_API_KEY は適宜置き換えてください）。
 * Access: Server (default)
 * 「Save & Test」をクリックして設定を保存します。

4. ダッシュボードの作成:

左側のメニューから「Create」（+アイコン）> 「Dashboard」を選択します。

「Add new panel」をクリックします。

Query設定で以下を入力します：
 * Data source: ThingSpeak
 * URL(Path): `/channels/YOUR_CHANNEL_ID/feeds.json`
 * (YOUR_CHANNEL_IDはThingSpeakのチャンネルIDに置き換えてください)
 * Method: GET
 * Params:
  TODO
        ```
        api_key: YOUR_READ_API_KEY
        results: 8000
        ```
        (YOUR_READ_API_KEYはThingSpeakのRead API Keyに置き換えてください)

「Transform」タブで、JSONデータを表形式に変換します：
 * Add transformation > JSON
 * Field name: feeds
 * Columns:
 TODO
        ```
        created_at
        field1
        field2
        field3
        field4
        field5
        field6
        ```

各フィールドの名前を適切なものに変更します（例：field1 → Temperature）。

グラフの種類や軸の設定を行います。時系列グラフの例：
 * X-axis: created_at
 * Y-axis: 表示したいフィールド（例：Temperature）

パネルの名前を設定し、「Apply」をクリックしてダッシュボードに追加します。

他のセンサーデータについても同様のパネルを作成します。

5. ダッシュボードの自動更新設定:

ダッシュボード右上の設定アイコンをクリックし、「Dashboard settings」を選択します。

「Time options」セクションで、適切な更新間隔を設定します。

6. ダッシュボードの保存:

   右上の「Save dashboard」ボタンをクリックしてダッシュボードを保存します。

7. ダッシュボードの共有（オプション）:

ダッシュボード上部の「Share dashboard」ボタンをクリックします。

「Link」タブでダッシュボードの公開URLを取得できます。

必要に応じて、認証設定やアクセス制限を行います。




//image[grafana][グラファナで作成したダッシュボード][scale=0.75]{
//}

筆者はネットワークカメラとしてSwitchbot社製のものを使っていますが、ネットワークカメラを使うことで、リアルタイムの生育状況を出先から確認することができます。

ラズベリーパイに安価なWebカメラを接続しGooglePhotoに送信し、タイムラプス動画を作成する方法も行なっていますが、この辺はソースコードが整理できしだい公開したいと思っています。
