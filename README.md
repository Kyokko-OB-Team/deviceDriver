# 自作DeviceDriver

testアプリのソースコードも参考にしてください。
<br>

## 目次

- [デバイスドライバを使うには](#デバイスドライバを使うには)
- [HC-SR04](#HC-SR04)
- [Build](#Build)

<br>
<br>

## デバイスドライバを使うには

デバイスドライバはカーネルモジュールとも呼ばれます。<br>
カーネルモジュールは、カーネルと同じソースからビルドする必要があるため、<br>
カーネルにあったデバイスドライバを使用する必要があります。<br>
カーネルバージョンを上げたり、カーネルソースを変更した場合は<br>
カーネルモジュールもビルドし直す必要がありますのでご注意ください。<br>
<br>
このカーネルモジュールが使用可能なカーネルイメージも本リポジトリにコミットしてあります。<br>
/bootディレクトリ配下のkernel.imgと差し替えることで、<br>
本リポジトリのカーネルモジュールをロードすることができるようになります。<br>
差し替えは、元のカーネルイメージをバックアップしてから行うのがおすすめです。<br>
<br>
また、カーネルイメージを更新した場合は、<br>
同時にビルドされたカーネルモジュールも合わせて更新する必要があります。<br>
<br>
本リポジトリをRaspberry Piにcloneした場合に、デバイスドライバを使用するまでの手順は以下を参考にしてください。<br>
microSDのみ別PCにマウントして書き換えを行う場合は、パスを適宜変更してください。<br>
<br>

```
$ cd ~/deviceDriver
$ sudo cp /boot/kernel.img /boot/kernel-bk.img
$ sudo cp ./kernel.img /boot/.
$ sudo tar --no-same-owner -xzvf ./dtb.tar.gz -C /boot/
$ sudo tar -xvjf ./modules.tar.bz2 -C /
$ sync
$ sudo shutdown -r now
```

<br>
カーネルモジュールは以下の方法で、ロード(カーネルに組み込む)することが出来ます。<br>

```
$ sudo insmod hc_sr04.ko
```

<br>
カーネルに組み込んだあとは、/devディレクトリ配下にデバイスファイルが作成されます。<br>
デバイスドライバを使用する場合は、こちらのデバイスファイルに対して操作を行います。<br>
<br>
カーネルからアンロードしたい(取り外したい)場合は以下のようにします。<br>

```
$ sudo rmmod hc_sr04
```

<br>
<br>


## HC-SR04

超音波距離センサで距離を取得するためのデバイスドライバです。<br>
コマンド `GPIO_HCSR04_EXEC_MEASURE_DISTANCE` で距離の計測要求を行い、<br>
`GPIO_HCSR04_GET_DISTANCE` で計測結果を取得します。<br>
<br>
センサの計測時間がありますので、<br>
`GPIO_HCSR04_EXEC_MEASURE_DISTANCE` から `GPIO_HCSR04_GET_DISTANCE` の実行までは<br>
1秒程度の時間を開けてください。<br>
<br>

| コマンド | データ | 方向 | 説明 |
|:---------|:------:|:----:|:-----|
| `GPIO_HCSR04_EXEC_MEASURE_DISTANCE` | - | out | 距離の測定要求 |
| `GPIO_HCSR04_GET_DISTANCE` | 距離データ(mm) | in | 測定した距離の取得 |

<br>
<br>

## Build

まずは対象のカーネルをビルドする必要があります。<br>
最低でも空き領域が10GB以上のストレージと8GB以上のメモリのPCで実施することをおすすめします。<br>
x86-64でクロスコンパイルします。<br>
<br>
また、カーネルをビルドし直す場合は、<br>
カーネルモジュールやデバイスツリーも更新する必要がありますのでご注意ください。<br>
<br>

```
// 必要パッケージのインストール
$ sudo apt install git bc bison flex libssl-dev make

// カーネルとクロスコンパイラをclone
$ git clone git@github.com:Kyokko-OB-Team/linux.git
$ git clone git@github.com:raspberrypi/tools.git

// 手元のRaspberry Pi ZeroのKernelと同じリビジョンでブランチ切ったのでチェックアウト
$ cd linux
$ git fetch origin raspbian-5.4.51_kernel
$ git checkout raspbian-5.4.51_kernel

// クロスコンパイルのための環境変数を設定
$ export ARCH=arm
$ export KERNEL=kernel

// .config作成
$ make bcmrpi_defconfig

// クロスコンパイラ指定
$ export CROSS_COMPILE=~/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-

// カーネルビルド
$ make zImage modules dtbs

// デバイスドライバのビルド
$ cd ./deviceDriver
$ . ./env.sh
$ cd ./hc-sr04
$ make
```

