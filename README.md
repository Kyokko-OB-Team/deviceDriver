# 自作DeviceDriver

testアプリのソースコードも参考にしてください。

## 目次

- [HC-SR04](#HC-SR04)


## HC-SR04

超音波距離センサで距離を取得するためのデバイスドライバです。<br>
コマンド `GPIO_HCSR04_EXEC_MEASURE_DISTANCE` で距離の計測要求を行い、<br>
`GPIO_HCSR04_GET_DISTANCE` で計測結果を取得します。<br>
<br>
センサの計測時間がありますので、<br>
`GPIO_HCSR04_EXEC_MEASURE_DISTANCE` から `GPIO_HCSR04_GET_DISTANCE` の実行までは<br>
1秒程度の時間を開けてください。<br>
<br>
<br>
| コマンド | データ | 方向 | 説明 |
|:---------|:------:|:----:|:-----|
| `GPIO_HCSR04_EXEC_MEASURE_DISTANCE` | - | out | 距離の測定要求 |
| `GPIO_HCSR04_GET_DISTANCE` | 距離データ(mm) | in | 測定した距離の取得 |




