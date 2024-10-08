﻿# 🎉aviutl用『コンボボックスパッチ』プラグイン

* [aviutl](https://spring-fragrance.mints.ne.jp/aviutl/)用のプラグインです。
* 設定ダイアログの切り替えを高速化します。

## ⬇️ダウンロード

* [最新バージョンはこちら](https://github.com/hebiiro/aviutl.combobox_patch.auf/releases/latest/)

## 🚀導入方法

1. pluginsフォルダをaviutlフォルダにコピーします。

## 💡使い方

1. aviutlのメニューから『表示』➡『コンボボックスパッチの表示』を選択します。
1. 『コンボボックスパッチ』ウィンドウが表示されます。

### 📝設定項目

* `コンボボックスを再構築しない`
	✏️再構築の必要がない場合はデフォルト処理をスキップします。
* `コンボボックスを再描画しない`
	✏️コンボボックスを再構築している間、描画を停止します。

## 🔖更新履歴

* r3 - 2024/08/18 💥大幅に仕様変更
* r2 - 2024/08/06 🩹オブジェクト分割を配置したときエラーが出る問題に対応
* r1 - 2024/08/05 🚀初版

## ⚗️動作確認

* [aviutl 1.10 & 拡張編集 0.92](https://spring-fragrance.mints.ne.jp/aviutl/)

## 💳クレジット

* [aviutl_exedit_sdk](https://github.com/ePi5131/aviutl_exedit_sdk)
* [Microsoft Research Detours Package](https://github.com/microsoft/Detours)

## 🐣発案

* [狐紺くろ(Tsut-ps)](https://x.com/Tsut_ps) / X
	* https://x.com/Tsut_ps/status/1721780757071626371

## 🤝協力

* r2
	* [かい⑨🍓](https://twitter.com/kai_maru9) / X
		* https://x.com/kai_maru9/status/1820446450440183978

## 💩開発者用

1. アニメーション効果のコンボボックスの構築自体が重い。
1. アニメーション効果のコンボボックスが二重に構築される。
	1. まず、全てのアイテム("震える...")が構築される。
	1. その後、カテゴリ毎のアイテムに再構築される。(例えば"描画@PSD..."など)

### 🚑️解決策

1. コンボボックスを再構築しない。
	1. 一度構築されたコンボボックスを配列の後ろの方にスワップしてキャッシュしておく。
	1. 必要になったらまた配列内をスワップしてコンボボックス自体を持ってくる。

## 👽️作成者情報

* 作成者 - 蛇色 (へびいろ)
* GitHub - https://github.com/hebiiro
* Twitter - https://x.com/io_hebiiro

## 🚨免責事項

この作成物および同梱物を使用したことによって生じたすべての障害・損害・不具合等に関しては、私と私の関係者および私の所属するいかなる団体・組織とも、一切の責任を負いません。各自の責任においてご使用ください。
