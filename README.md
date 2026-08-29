# 🌙 Fajer Alsamnah

## منبّه إسلامي ذكي لأوقات الصلاة باستخدام ESP32

**A smart Islamic prayer alarm system built with ESP32.**

![صورة المشروع](images/fajer-alsamnah.jpeg)

## 📌 نبذة عن المشروع | About

**Fajer Alsamnah** هو مشروع **IoT** يعتمد على ESP32 وBuzzer لإنشاء منبّه ذكي لأوقات الصلاة، مع التركيز على مساعدة المستخدم على الاستيقاظ لصلاة الفجر.

**Fajer Alsamnah** is an **IoT project** based on ESP32 and a Buzzer, designed to provide a smart alarm for prayer times, with a focus on helping users wake up for Fajr prayer.

يحصل النظام تلقائيًا على أوقات الصلاة من **Prayer Times API** عبر الإنترنت، ثم يقوم بتشغيل المنبّه عند حلول وقت الصلاة المحددة.

The system automatically retrieves prayer times using a **Prayer Times API** and activates the alarm when the selected prayer time arrives.

## 🎯 المشكلة | Problem

تتغير أوقات الصلاة يوميًا، مما يجعل ضبط المنبّه يدويًا أمرًا مزعجًا وقد يؤدي إلى نسيان تحديث الوقت.

Prayer times change every day, making manual alarm adjustment inconvenient and easy to forget.

## 💡 الحل | Solution

يقوم ESP32 بالحصول على أوقات الصلاة تلقائيًا، ومزامنة الوقت باستخدام **NTP**، ثم مراقبة أوقات الصلاة وتشغيل الـBuzzer عند الموعد المحدد.

The ESP32 automatically retrieves prayer times, synchronizes the time using **NTP**, and activates the Buzzer when the selected prayer time arrives.

```text
Prayer Times API
       ↓
      Wi-Fi
       ↓
      ESP32
       ↓
Alarm Controller
       ↓
     Buzzer
```

## ⭐ المميزات | Features

- 🕌 دعم الصلوات الخمس | Supports the five daily prayers
- ⏰ منبّه تلقائي للفجر وبقية الصلوات | Automatic prayer alarms
- 🌐 تحديث أوقات الصلاة عبر الإنترنت | Online prayer time updates
- 📱 واجهة Web للتحكم من الهاتف | Web interface for mobile control
- 🔔 تشغيل/إيقاف المنبّه لكل صلاة | Individual alarm control
- ⏱️ تحديد مدة المنبّه | Adjustable alarm duration
- 🎵 اختيار نغمة المنبّه | Multiple alarm tones
- 📍 تحديد الموقع باستخدام Latitude & Longitude | Location-based prayer times
- 🕐 مزامنة الوقت باستخدام NTP | NTP time synchronization

## 📱 واجهة التحكم | Web Interface

يمكن الوصول إلى واجهة التحكم من الهاتف المتصل بنفس شبكة Wi-Fi.

The control interface can be accessed from a phone connected to the same Wi-Fi network.

```text
http://ESP32-IP
```

مثال | Example:

```text
http://192.168.1.100
```

## 🔌 المكونات | Hardware

- ESP32
- Buzzer
- USB Cable
- Wi-Fi Network

### توصيل Buzzer | Buzzer Wiring

| Buzzer | ESP32 |
|---|---|
| (+) | GPIO 25 |
| (-) | GND |

![ESP32 Buzzer Wiring](images/esp32-buzzer-wiring.png)

## 📍 الموقع الافتراضي | Default Location

```text
City: Aden
Country: Yemen

Latitude: 12.785496
Longitude: 45.018654
```

يمكن تغيير الإحداثيات لاستخدام المشروع في مدينة أخرى.

The coordinates can be changed to use the system in another location.

## ⚙️ طريقة التشغيل | Setup & Run

### 1. المتطلبات | Requirements

- Arduino IDE
- ESP32 Board Package
- ArduinoJson Library
- ESP32 Board
- Buzzer
- Wi-Fi

### 2. إعداد Wi-Fi | Wi-Fi Configuration

أدخل بيانات شبكة Wi-Fi في الكود:

Enter your Wi-Fi credentials:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

### 3. رفع الكود | Upload

اختر:

```text
Board: ESP32 Dev Module
```

ثم اختر منفذ **COM** واضغط **Upload**.

Open **Serial Monitor** على سرعة:

```text
115200 Baud
```

بعد ظهور عنوان IP، افتحه من الهاتف.

After the IP address appears, open it from your phone.

> ⚠️ يجب أن يكون الهاتف وESP32 متصلين بنفس شبكة Wi-Fi.
>
> ⚠️ The phone and ESP32 must be connected to the same Wi-Fi network.

## 🛠️ التقنيات المستخدمة | Technologies

- ESP32
- Arduino / C++
- Wi-Fi
- REST API
- JSON
- NTP
- Web Server
- HTML / CSS / JavaScript
- Buzzer

## 🎓 الهدف | Purpose

يهدف المشروع إلى تطبيق مفاهيم **IoT وEmbedded Systems** في نظام عملي منخفض التكلفة يساعد المستخدم على متابعة أوقات الصلاة والاستيقاظ لصلاة الفجر.

The project aims to apply **IoT and Embedded Systems** concepts in a low-cost practical system that helps users stay on time for prayers and wake up for Fajr.

## 🚀 التطوير المستقبلي | Future Development

- OLED Display
- RTC Module
- Battery Backup
- دعم عدة مدن | Multi-city support
- Offline Prayer Times
- Mobile Application
- Speaker بدلاً من Buzzer

## ⚠️ ملاحظة | Note

يحتاج النظام إلى اتصال بالإنترنت للحصول على أوقات الصلاة وتحديثها.

The system requires an Internet connection to retrieve and update prayer times.

## 👨‍💻 المطور | Developer

**Abdullah Cyber**

Cybersecurity Student & Developer

---

> 🌙 **صلِّ في وقتك، واستيقظ للفجر بسهولة.**
>
> **Pray on time. Wake up for Fajr with ease.**
