# ecc-for-linux
<h1> Excalibur Control Center Uygulamasının Linux Portu. </h1>

<h2> Excalibur G870 model bir laptopu tersine mühendislikle inceledikten sonra geliştirdiğim uygulamadır.</h2>

<h1> 2026'nın Temmuz ayı itibariyle ekran kartı değiştirme desteği gelmiştir. Fakat her yazılımda olduğu gibi, çalıştığına dair garanti vermiyorum.</h1>

<h3> Eğer Arch Linux kullanıyorsanız "make" ve "linux-headers", Debian kullanıyorsanız "make" ve "linux-headers-amd64" paketlerini indirdiğinizden emin olun. Aksi takdirde ekran kartı değişimi çalışmayacaktır.</h3>

![Türkçe arayüz](https://raw.githubusercontent.com/FeusX/ecc-for-linux/main/ecc.png)

```g++ main.cc components/keyboard/keyboard.cc components/ui/ui.cc -o components/gpu/optimus.cc ecc -pthread $(pkg-config --cflags --libs gtk4)``` compile with this command.
