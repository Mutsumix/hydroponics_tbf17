= IoTを活用したモニタリング

収集して得たさまざまな値は多くの知見を与えてくれます。

この章ではラズベリーパイと環境センサを用いた値の取得方法について解説します。

ただデータが取れるだけでは面白くないので、それを外部に飛ばして可視化する方法についても紹介します。

筆者は比較的簡単な手順でかつ無料に使用できるThingSpeakというサービスを使っています。

ThingSpeakで得たデータをGrafanaで参照し、ダッシュボードを作っているので、それについても合わせて触れます。@<fn>{automation}

//footnote[automation][理想はセンサーの値によって給水をしたり温度を調整するといったフローの自動化ができればよいのですが、今後の課題です]

== 用意するもの

 * Raspberry Pi@<fn>{raspberry_pi}
 * 環境センサ
 ** Omronから発売されている 2JCIE-BU01（F1） という機種を使用します。
 ** これをラズパイにUSB接続します。ちなみに公式アプリをアプリストアからダウンロードすれば、特に何の設定もすることなく、スマホからデータの確認が可能です。@<fn>{end_of_service}

//image[omron_app1][アプリ画面その１][scale=0.5]
//image[omron_app2][アプリ画面その２][scale=0.5]
//footnote[raspberry_pi][Raspberry Pi 4 Model Bで動作確認しています。前はもっと安かった気がしますが、今一万円以上するみたいです。]
//footnote[end_of_service][しかし2025年9月でアプリのサポートが終了するようです。機器本体も2025年1月で生産終了予定...]

== 環境センサを使用するための準備
まず、環境センサをラズパイにUSBで繋ぎます。@<fn>{bluetooth}

//footnote[bluetooth][bluetoothでの接続は筆者の環境ではうまくいかなかったのでUSBで接続してデータを取得する方法を取っています]

=== 環境センサを使用するにあたっての操作

プログラムから環境センサを使用するためには、センサとドライバーを関連付ける必要があります。@<fn>{usb_driver}

//footnote[usb_driver][こちらの記事を全面的に参考にしています。@<href>{https://qiita.com/Toshiaki0315/items/aa43e78c024bb900ef53, オムロン環境センサ（2JCIE-BU）をラズパイで使ってみた。（１）}（https://qiita.com/Toshiaki0315/items/aa43e78c024bb900ef53）]

こちらの記事を全面的に参考にしています。

環境センサをUSB接続した状態で、FTDIのドライバと2JCIE-BUを関連付けます。

//cmd{
$ lsusb
//}

まず、lsusbコマンドでデバイス情報を確認し、次のようにOmronのデバイスが表示されればOKです。

//list[lsusb_output][lsusbコマンドの出力][scale=0.75]{
Bus 001 Device 002: ID 0590:00d4 Omron Corp.
//}


続いて、こちらのコマンドでFTDIのドライバと2JCIE-BUを関連付けます。

//cmd{
$ sudo modprobe ftdi_sio
$ sudo sh -c "echo 0590 00d4 > /sys/bus/usb-serial/drivers/ftdi_sio/new_id"
//}

確認はこちらのコマンドで行います。

//cmd{
$ dmesg | grep USB
//}

一番最後の行でFTDI USB Serial Device converterがttyUSB0として認識されていればOKです。

//list[dmesg_output][dmesgコマンドの出力（最終行）][scale=0.75]{
usb 1-1.4: FTDI USB Serial Device converter now attached to ttyUSB0
//}

仕上げとして、再起動のたびに毎回前述のコマンドを打たなくても済むように、2JCIE-BUを自動認識するように設定します。@<fn>{vi}

//footnote[vi][viに慣れていない方はnanoを使ってね。sudo nano /etc/udev/rules.d/80-2jcie-bu01-usb.rules]

//cmd{
$ sudo vi /etc/udev/rules.d/80-2jcie-bu01-usb.rules
//}

このファイルに次の3行を記述します。

//list[udev_rules][udevのルール][scale=0.75]{
ACTION=="add", \
SUBSYSTEMS=="usb", \
ATTRS{idVendor}=="0590", ATTRS{idProduct}=="00d4", \
RUN+="/bin/sh -c '/sbin/modprobe ftdi_sio && echo 0590 00d4 > @<embed>$|latex|\linebreak\hspace*{5ex}$/sys/bus/usb-serial/drivers/ftdi_sio/new_id'"
//}

保存をしたら、udevに新しいルールを適用します。

//cmd{
$ sudo udevadm control --reload
//}

以上でセンサーの準備は完了です。

== サンプルコードの紹介
筆者が作成して使用している、ThingSpeakへのデータ送信のためのサンプルリポジトリを紹介します。

ThingSpeakというサービスを知らない方に簡単に説明すると、IoTデータを可視化するためのサービスです。@<fn>{thingspeak}

//footnote[thingspeak][本当はGoogleスプレッドシートへのデータ送信を行なっていたのですが、アカウント作成から手順化するのが面倒だったので、方法を変えました。しかし、この方が安定してGrafanaからのデータ参照ができるし、コードもシンプルになったので結果オーライです]

=== プログラム実行前の準備（ThingSpeak側）

プログラムを実行するには、ThingSpeakのチャンネルIDの設定とAPIキーの取得が必要になるので、ThingSpeakアカウントの作成手順からチャンネルの設定までを説明します。

データの受け手となるThingSpeak（@<href>{https://thingspeak.mathworks.com/}）のアカウントを作成します。

//image[thingspeak][ThingSpeakのトップ画面][scale=0.75]

登録にはメールアドレスが必要なので、登録と認証を済ませます。

//image[thingspeak_signup][アカウント登録画面][scale=0.75]

登録が完了すると、マイページが表示されるので「New Channel」をクリックします。

//image[thingspeak_new_channel][チャンネル作成ボタン][scale=0.75]

チャンネル名と、どういった値を受信するのかを設定する必要があるので、次のように入力します。

//image[thingspeak_channel_settings][チャンネル設定画面][scale=0.75]

最後に、「Save Channel」をクリックします。

//image[thingspeak_channel_save][チャンネル保存ボタン][scale=0.75]

チャンネルが作成されると、各種タブが表示されるので、「API Keys」タブをクリックし、Read API KeysとWrite API Keyが存在するのを確認します。

//image[thingspeak_api_keys][API Keysタブ][scale=0.75]
//image[thingspeak_api_keys_values][Read API KeysとWrite API Keyの値][scale=0.75]

これで、ラズパイ側でプログラムを実行し、ThingSpeakにデータを送信する準備が整いました。

=== プログラム実行前の準備（ラズパイ側）

まず、ラズパイ上にGitHubから@<href>{https://github.com/Mutsumix/RasPi-EnvSensor-ThingSpeak, https://github.com/Mutsumix/RasPi-EnvSensor-ThingSpeak}のリポジトリをクローンし、続いて、必要なライブラリをインストールします。

//cmd{
$ cd RasPi-EnvSensor-ThingSpeak
$ pip install -r requirements.txt
//}

次に設定ファイルをコピーして編集します。

//cmd{
$ cp config.yml.example config.yml
$ nano config.yml
//}

config.ymlで設定できるのは次の項目です。

 * api_key: ThingSpeakのWrite API キー
 * interval_minutes: データの取得頻度（分単位）
 * port: 環境センサのポート番号

//list[config.yml][config.yml][scale=0.75]{
# ThingSpeak設定
thingspeak:
  api_key: "YOUR_THINGSPEAK_WRITE_API_KEY"  # ThingSpeakのWrite API Key

# センサー設定
sensor:
  scheduler:
    # データ送信の間隔（分）
    interval_minutes: 60
  omron:
    use: True
    port: "/dev/ttyUSB0"  # センサーのシリアルポート（環境に合わせて変更してください）
//}

=== プログラムの実行

プログラムの実行は次のコマンドで行えます。

//cmd{
$ python main.py
//}

設定した頻度でデータが取得され、値がThingSpeakに書き込まれていることを確認します。

//image[thingspeak_data_view][データの確認][scale=0.75]

=== Grafanaでダッシュボードを作成する
これでセンサーの値をWeb上で確認できるようになりましたが、筆者はGrafanaにダッシュボードを作成しています。
こうすれば、視覚的に今どうなっているのか把握しやすいですし、外出先でも簡単に生育環境の状況を確認することができます。

1. Grafana Cloudアカウントの作成:

Grafana Cloudのウェブサイト（@<href>{https://grafana.com/products/cloud/}）にアクセスします。

「Create free account」をクリックし、指示にしたがってアカウントを作成します。ソーシャルログインに対応しているので簡単にログインが可能です。

//image[grafana_cloud_signup][Grafana Cloudのアカウント登録画面][scale=0.75]

アカウント作成後、セットアップ画面が表示されます。右上の「I'm already familiar with Grafana〜」をクリックし、この画面はスキップします。

//image[grafana_cloud_setup][セットアップ画面][scale=0.75]

2. Connectionの追加

「Add new connection」と画面に表示されるのを確認します。もし表示されていなければ左側のメニューから「Connections」> 「add new connection」を選択します。

#@# 「Add data source」をクリックします。？

検索バーで 「JSON」 と検索し、「JSON API」を選択します。

//image[grafana_cloud_add_json_api][JSON APIの選択画面][scale=0.75]

「install」を選択します。

//image[grafana_cloud_install_json_api][JSON APIのインストール画面][scale=0.75]

3. Data Sourceの追加

左側のメニューから「Connections」> 「Data sources」を選択します。

//image[grafana_cloud_data_sources][Data sources画面][scale=0.75]

「Add new data source」をクリックし、先ほどインストールしたJSON APIを選択します。

//image[grafana_cloud_add_data_source][Data sourceの追加画面][scale=0.75]
//image[grafana_cloud_data_source_select][JSON APIの選択][scale=0.75]

次の設定を行います。

 * Name: ThingSpeak（任意の名前です）
 * URL: https://api.thingspeak.com/channels/@<b>{ID}/feeds.json?api_key=@<b>{API} @<fn>{thingspeak_api_url}

//image[grafana_cloud_data_source_settings][Data sourceの設定画面][scale=0.75]

 * ここまでの設定に問題がなければ「Save & Test」をクリックすると、画面上にSuccessと表示され、設定が保存されます。

//image[grafana_cloud_data_source_success][Data sourceの設定成功画面][scale=0.75]

//footnote[thingspeak_api_url][ID と API はThingSpeakのチャンネルIDとRead API Keyにそれぞれ置き換えてください]

4. ダッシュボードの作成:

左側のメニューから「Dashboard」を選択します。

//image[grafana_cloud_create_dashboard][Dashboardの作成画面][scale=0.75]

「New dashboard」をクリックします。

//image[grafana_cloud_add_dashboard][Panelの追加画面][scale=0.75]

「Add visualization」をクリックします。

//image[grafana_cloud_add_visualization][Visualizationの追加画面][scale=0.75]

データソースを聞かれるので、先ほど作成したThingSpeakを選択するために検索窓に「Thing」と入力し、「ThingSpeak」を選択します。

//image[grafana_cloud_select_data_source][Data Sourceの選択画面][scale=0.75]

データソースの詳細設定を行う画面が表示されるので、設定を行います。

次の２つを設定します（@<list>{grafana_cloud_data_source_settings_fields}）。

//list[grafana_cloud_data_source_settings_fields][Data Sourceの設定画面][scale=0.75]{
$. feeds [*].created_at
$. feeds [*].field1
//}

また、型[Type]を正しく設定する必要があるため、次のように設定します（@<list>{grafana_cloud_data_source_settings_fields_type}）。

//list[grafana_cloud_data_source_settings_fields_type][Data Sourceの設定画面][scale=0.75]{
$. feeds [*].created_at -> Time
$. feeds [*].field1 -> Number
//}

//image[grafana_cloud_data_source_settings2][Data Sourceの設定画面][scale=0.75]

すると画面上に取得した数値が折れ線グラフで表示されます。

あとは、パネルのTitleを適切なものに変更します。Field1の場合は、気温のため「Temperature」もしくは「気温の推移」というように変更します。

Save Dashboardをクリックしてダッシュボードを保存します。

同様の手順で、他のセンサーデータについても取得できるようにしパネルを作成します。

Grafanaでは表示形式について色々と設定できますが、それについて説明すると長くなってしまうのと、筆者自身が詳しくないため、割愛します。

参考までに筆者はこちらの図のようなダッシュボードを作成しています。

//image[grafana_cloud_dashboard][筆者のダッシュボード][scale=0.75]

== カメラによる監視

また、筆者はネットワークカメラとしてSwitchbot社製のものを使っていますが、ネットワークカメラを使うことで、リアルタイムの生育状況を出先から確認することができます。

//image[switchbot_camera][Switchbotのネットワークカメラで撮影した画像][scale=0.75]

ラズベリーパイに安価なWebカメラを接続しGooglePhotoに送信し、タイムラプス動画を作成する方法も行なっていますが、この辺はソースコードが整理できしだい公開したいと思っています。

//image[raspberry_pi_camera][ラズベリーパイに接続したWebカメラで撮影した画像][scale=0.75]

カメラについては試行錯誤中です。

//image[switchbot_and_raspberry_pi_camera][カメラについてはまだ検証中です][scale=0.75]
