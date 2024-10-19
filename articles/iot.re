= IoTを活用したモニタリング

収集して得たさまざまな値は多くの知見を与えてくれます。

この章ではラズベリーパイと環境センサーを用いた値の取得方法について解説します。

ただデータが取れるだけでは面白くないので、それを外部に飛ばして可視化する方法についても触れます。

比較的簡単な手順でかつ無料に使用できるThingSpeakというサービスを使っています。

筆者はThingSpeakで得たデータをGrafanaでデータを参照し、ダッシュボードを作っているので、それについても合わせて触れます。（理想はセンサーの値によって給水をしたり温度を調整するといったフローの自動化ができれば良いのですが、今後の課題です）

== 用意するもの

 * ラズベリーパイ（4B）@<fn>{raspberry_pi}
 * 環境センサ
 ** Omronから発売されている 2JCIE-BU01(F1) という機種になります。
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

=== プログラム実行前の準備（ThingSpeak側）

データの受け手となるThingSpeak(@<href>{https://thingspeak.mathworks.com/})のアカウントを作成します。

//image[thingspeak][ThingSpeakのトップ画面][scale=0.5]

登録にはメールアドレスが必要なので、登録と認証を済ませます。

//image[thingspeak_signup][アカウント登録画面][scale=0.5]

登録が完了すると、マイページが表示されるので「New Channel」をクリックします。

//image[thingspeak_new_channel][チャンネル作成ボタン][scale=0.5]

チャンネル名とどういった値を受信するのかを設定する必要があるので、以下のように入力します。

//image[thingspeak_channel_settings][チャンネル設定画面][scale=0.5]

最後に、「Save Channel」をクリックします。

//image[thingspeak_channel_save][チャンネル保存ボタン][scale=0.5]

チャンネルが作成されると、各種タブが表示されるので、「API Keys」タブをクリックし、Read API KeysとWrite API Keyが存在するのを確認します。

//image[thingspeak_api_keys][API Keysタブ][scale=0.5]
//image[thingspeak_api_keys_values][Read API KeysとWrite API Keyの値][scale=0.5]

これで、ラズパイ側でプログラムを実行し、ThingSpeakにデータを送信する準備が整いました。

=== プログラム実行前の準備（ラズパイ側）

まず、GitHubから@<href>{https://github.com/Mutsumix/RasPi-EnvSensor-ThingSpeak, https://github.com/Mutsumix/RasPi-EnvSensor-ThingSpeak}のリポジトリをラズパイ上にクローンします。

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

=== プログラムの実行

プログラムの実行は以下のコマンドで行えます。

//cmd{
$ python main.py
//}

設定した頻度でデータが取得され、値がThingSpeakに書き込まれていることを確認します。

//image[thingspeak_data_view][データの確認][scale=0.5]

=== Grafanaでダッシュボードを作成する
これで終了でも良いのですが、あまり面白くないので、私はGrafabaにダッシュボードを作成しています。
こうすれば、視覚的に今どうなっているのか把握しやすいですし、外出先でも簡単に生育状況を確認することができます。

1. Grafana Cloudアカウントの作成:

Grafana Cloudのウェブサイト（@<href>{https://grafana.com/products/cloud/}）にアクセスします。

"Create free account"をクリックし、指示に従ってアカウントを作成します。ソーシャルログインに対応しているので簡単にログインが可能です。

//image[grafana_cloud_signup][Grafana Cloudのアカウント登録画面][scale=0.5]

アカウント作成後、セットアップ画面が表示されます。右上の「I'm already fimiliar with Grafana」をクリックし、この画面はスキップします。

//image[grafana_cloud_setup][セットアップ画面][scale=0.5]

2. Connectionの追加

「Add new connection」と画面に表示されるのを確認します。もし表示されていなければ左側のメニューから「Connections」> 「add new connection」を選択します。

#@# 「Add data source」をクリックします。？

検索バーで "JSON" と検索し、「JSON API」を選択します。

//image[grafana_cloud_add_json_api][JSON APIの選択画面][scale=0.5]

installします

//image[grafana_cloud_install_json_api][JSON APIのインストール画面][scale=0.5]

3. Data Sourceの追加

左側のメニューから「Connections」> 「Data sources」を選択します。

//image[grafana_cloud_data_sources][Data sources画面][scale=0.5]

Add new data sourceをクリックし、先ほどインストールしたJSON APIを選択します。

//image[grafana_cloud_add_data_source][Data sourceの追加画面][scale=0.5]
//image[grafana_cloud_data_source_select][JSON APIの選択][scale=0.5]

以下の設定を行います：
 * Name: ThingSpeak（任意の名前です）
 * URL: https://api.thingspeak.com/channels/CHANNEL_ID/feeds.json?api_key=YOUR_API_KEY @<fn>{thingspeak_api_url}

//image[grafana_cloud_data_source_settings][Data sourceの設定画面][scale=0.5]

 * ここまでの設定に問題がなければ「Save & Test」をクリックすると、画面上にSuccessと表示され、設定が保存されます。

//image[grafana_cloud_data_source_success][Data sourceの設定成功画面][scale=0.5]

//footnote[thingspeak_api_url][CHANNEL_ID と YOUR_API_KEY はThingSpeakのチャンネルIDとRead API Keyに置き換えてください]

4. ダッシュボードの作成:

左側のメニューから「Create」（+アイコン）> 「Dashboard」を選択します。

//image[grafana_cloud_create_dashboard][Dashboardの作成画面][scale=0.5]

「New dashboard」をクリックします。

//image[grafana_cloud_add_dashboard][Panelの追加画面][scale=0.5]

「Add visualization」をクリックします。

//image[grafana_cloud_add_visualization][Visualizationの追加画面][scale=0.5]

データソースを聞かれるので、先ほど作成したThingSpeakを選択するために検索窓に「Things」と入力し、「ThingSpeak」を選択します。

//image[grafana_cloud_select_data_source][Data Sourceの選択画面][scale=0.5]

データソースの詳細設定を行う画面が表示されるので、以下のように設定します。

以下の２つを設定します。

$. feeds [*].created_at
$. feeds [*].field1

また、型[Type]を正しく設定する必要があるため、以下のように設定します。

$. feeds [*].created_at -> Time
$. feeds [*].field1 -> Number

//image[grafana_cloud_data_source_settings2][Data Sourceの設定画面][scale=0.5]

すると画面上に取得した数値が折れ線グラフで表示されます。

あとは、パネルのTitleを適切なものに変更します。Field1の場合は、気温のため「Temperature」もしくは「気温の推移」というように変更します。

Save Dashboardをクリックしてダッシュボードを保存します。

同様の手順で、他のセンサーデータについても取得できるようにしパネルを作成します。

Grafanaでは表示形式について色々と設定できますが、それについて説明すると長くなってしまうのと、筆者自身が詳しくないため、割愛します。

参考までに筆者は以下のようなダッシュボードを作成しています。

//image[grafana_cloud_dashboard][筆者のダッシュボード][scale=0.5]


== カメラによる監視

また、筆者はネットワークカメラとしてSwitchbot社製のものを使っていますが、ネットワークカメラを使うことで、リアルタイムの生育状況を出先から確認することができます。

//image[switchbot_camera][Switchbotのネットワークカメラで撮影した画像][scale=0.5]

ラズベリーパイに安価なWebカメラを接続しGooglePhotoに送信し、タイムラプス動画を作成する方法も行なっていますが、この辺はソースコードが整理できしだい公開したいと思っています。

//image[raspberry_pi_camera][ラズベリーパイに接続したWebカメラで撮影した画像][scale=0.5]

カメラについては試行錯誤中です。

//image[switchbot_and_raspberry_pi_camera][カメラについてはまだ検証中です][scale=0.5]
