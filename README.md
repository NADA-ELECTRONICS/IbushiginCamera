# IbushiginCamera
IbushiginCamera for GR-LYCHEE - OpenCV

# いぶし銀カメラ
2018/02/24 [FlashAir Developers Summit 2018](https://flashair-developers.com/ja/about/events/summit2018/) 場所:LODGE - Yahoo Japan
[GR-LYCHEE - Mbed](https://os.mbed.com/platforms/Renesas-GR-LYCHEE/)  
[GR-LYCHEE - GADGET RENESAS](https://os.mbed.com/platforms/Renesas-GR-LYCHEE/)  
[GR-LYCHEE - CORE](http://www.core.co.jp/product/m2m/gr-lychee/)  
[AS-289R2 Thermal Printer Shield - NADA ELECTRONICS](http://www.nada.co.jp/as289r2/)  
[FlashAir W-04 - TOSHIBA](https://flashair-developers.com/ja/discover/overview/w04/)  

![IbushiginCamera](https://github.com/NADA-ELECTRONICS/IbushiginCamera/blob/master/photo.jpg)

# 接続
GR-LYCHEE (D13) - シャッターSW  
GR-LYCHEE (D1)  - (RxD) AS-289R2  

# 主な処理
シャッターSWが押されるとカメラで取得したグレースケール画像cv::Mat dstにコピー
グレースケール画像サイズをcv:resizeで384xautoに変換
cv::imwriteでxxx_gray.bmpをFlashAirに保存
cv::Mat dstをディザリング処理で2値化
cv::flipで反転(デモが反転印字の為)
cv::imwriteでxxx_color2.bmpをFlashAirに保存
FlashAirのxxx_color2.bmpファイルを読み出しながらAS-289R2 Thermal Printerにuart送信

# 今後
FlashAirをSD-Cardとしか利用していないので、ゴニョゴニュしてみたいと思います。
