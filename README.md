# ecc-for-linux
<h1> Excalibur Control Center Uygulamasının Linux Portu. </h1>

<h2>Excalibur G870 laptop'umun anakartını tersine mühendislik yoluyla inceledikten sonra programladım. Sorunsuz çalışmaktadır fakat hala GPU kontrolü bulunmamaktadır. Uygulamanın arayüzü aşağıdaki gibidir. Son uyguladığınız renge göre 3d ızgara renk değiştirmektedir. Türkçe ve İnglizce olmak üzere iki adet dil seçeneği mevcuttur.</h2>

<h3>GPU kontrolü eklediğimde daha iyi belgelendireceğim.</h3>

![Türkçe arayüz](https://raw.githubusercontent.com/FeusX/ecc-for-linux/main/ecc.png)

```g++ main.cc components/keyboard/keyboard.cc components/ui/ui.cc -o ecc -pthread $(pkg-config --cflags --libs gtk4) -O2``` compile with this command.
