## ESP32 CLAW MACHINE
Welcome!
Im gonna try and log here the steps I'm taking or took into the making of this project (_hopefully to completion_).

# Demo
_*Demo showing, am I rite?*_

## Materials.
### (_project thingies_)
- [Claw Machine Ali](https://es.aliexpress.com/wholesale?SearchText=claw%20machine)
- [ESP32 Ali](https://es.aliexpress.com/wholesale?SearchText=esp32) / [ESP32 Ama](https://www.amazon.com/s?k=esp32)
- 3 x [H Bridge Ali](https://es.aliexpress.com/wholesale?SearchText=H+bridge+arduino) / [H Bridge Ama](https://www.amazon.com/s?k=h+bridge+arduino)

## First look at what this __claw machine__ does.
### (_so far, It works_)
[From Ali](https://es.aliexpress.com/item/1005003390056010.html?spm=a2g0o.productlist.0.0.78aa4144hfqjZs&algo_pvid=c3c81393-2dcb-4ee8-90cb-f53fe9cc25b0&algo_exp_id=c3c81393-2dcb-4ee8-90cb-f53fe9cc25b0-22&pdp_ext_f=%7B%22sku_id%22%3A%2212000025557493513%22%7D)
![Claw Machine](/Imgs/Claw Machine.jpg)
What can we see?
- 3 x "Joysticks". _fair enough_

## Undoing someone's hard work
### (_What's inside? unfixing what was a working claw machine_)
I grab a screwdriver and do what everyone does best, take apart everything, and hope I remember where all the screws go back in.
I focus on the claw's movement, because that's the mayor __plot__.

So.. what's on display now?
- 1 x Button (check coin input).
- 1 x Led, after coin input.
- 1 x Speaker. _an annoying one, tbh..._
- 3 x __swtiches__ (used as joysticks).
- 3 x Motors.

![Circuitry](/Imgs/Circuitry 01.jpg)
![Circuitry](/Imgs/Circuitry 02.jpg)
![Circuitry](/Imgs/Circuitry 03.jpg)
![Circuitry](/Imgs/Circuitry 04.jpg)
![Circuitry](/Imgs/Circuitry 05.jpg)
![Circuitry](/Imgs/Circuitry 06.jpg)
![Switch](/Imgs/Switch 1.jpg)
![Switch](/Imgs/Switch 2.jpg)

Ok, now we can think about tinkering.

## Doin' smth
Bypass switches to motors...

## Bibliography
- [randomnerdtutorials OTA](https://randomnerdtutorials.com/esp32-ota-over-the-air-arduino/)
- [randomnerdtutorials Websocket](https://randomnerdtutorials.com/esp32-websocket-server-arduino/)
- [randomnerdtutorials GPIOS](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [yoannmoi Joysticks](https://yoannmoi.net/nipplejs/)
- [HTML Joystick](https://moshfeu.medium.com/how-to-build-an-html5-games-controller-with-arduino-nodejs-and-socket-io-part-2-bbd01bf36481)
- [m1cr0lab-esp32 websocket](https://m1cr0lab-esp32.github.io/remote-control-with-websocket/websocket-and-json/)

------------------------------------

Whenever you commit to this repository, GitHub Pages will run [Jekyll](https://jekyllrb.com/) to rebuild the pages in your site, from the content in your Markdown files.

### Markdown

Markdown is a lightweight and easy-to-use syntax for styling your writing. It includes conventions for

```markdown
Syntax highlighted code block

# Header 1
## Header 2
### Header 3

- Bulleted
- List

1. Numbered
2. List

**Bold** and _Italic_ and `Code` text

[Link](url) and ![Image](src)
```

For more details see [Basic writing and formatting syntax](https://docs.github.com/en/github/writing-on-github/getting-started-with-writing-and-formatting-on-github/basic-writing-and-formatting-syntax).

### Jekyll Themes

Your Pages site will use the layout and styles from the Jekyll theme you have selected in your [repository settings](https://github.com/Jtroncho/Claw-Machine-ESP32/settings/pages). The name of this theme is saved in the Jekyll `_config.yml` configuration file.

### Support or Contact

Having trouble with Pages? Check out our [documentation](https://docs.github.com/categories/github-pages-basics/) or [contact support](https://support.github.com/contact) and we’ll help you sort it out.
