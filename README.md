# About

Learning the book, [［試して理解］Linuxのしくみ～実験と図解で学ぶOSとハードウェアの基礎知識](http://gihyo.jp/book/2018/978-4-7741-9607-7)

See also [satoru-takeuchi/linux-in-practice](https://github.com/satoru-takeuchi/linux-in-practice)

# Building and running the codes

All codes in the labs directory are supposed to be compiled and run on the Linux platform.
To run the codes on non-Linux machines, install Vagrant and,

```bash
$ vagrant up --provision
$ vagrant ssh
```

To compile C codes,

```bash
$ cd labs/chapter4  # for example
$ make
```
