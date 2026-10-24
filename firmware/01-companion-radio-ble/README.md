# MeshCore 1.18 — Companion Radio (BLE + Wi-Fi)

[English](#english) | [Polski](#polski)

---

<a id="english"></a>
## English

Firmware role for **M5Stack Unit C6L** that connects over Bluetooth Low Energy (BLE), USB Web Serial, or local 2.4GHz Wi-Fi (TCP port 5000) to mobile apps (e.g. MeshCore App / Web Flasher).

### Binary Details
- **Build Target**: `M5Stack_Unit_C6L_companion_radio_ble`
- **Firmware Size**: ~2,120 KB
- **RAM Usage**: 58.6% (192 KB / 328 KB)
- **Flash Usage**: 66.7% (2,098 KB / 3,145 KB) via `huge_app.csv`
- **Bluetooth PIN**: Random 6-digit PIN on boot (displayed on screen)
- **Wi-Fi TCP Server**: Active on TCP port `5000`
- **Flash Mode**: `DIO` (80 MHz)

### Files
- `firmware.bin` — Main application image (flash offset `0x10000`)
- `bootloader.bin` — ESP32-C6 bootloader (flash offset `0x0`)
- `partitions.bin` — Partition table (flash offset `0x8000`)

### Flashing Instructions

#### Standard Flash (Retains settings)
Double-click `scripts\flash-companion.bat` or run:
```powershell
..\..\scripts\flash-companion.ps1
```

#### Clean Flash (Recommended — Erases old SPIFFS / pairings first)
Double-click `scripts\flash-companion-clean.bat` or run:
```powershell
..\..\scripts\flash-companion.ps1 -Erase
```

#### Full Flash Erase Only
Double-click `scripts\erase-flash.bat` or run:
```powershell
..\..\scripts\erase-flash.ps1
```

Or directly using `esptool.py`:
```bash
# 1. Clean wipe (recommended)
python esptool.py --chip esp32c6 --baud 921600 erase_flash

# 2. Flash firmware
python esptool.py --chip esp32c6 --baud 921600 write_flash 0x0 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
```

---

### Wi-Fi Configuration & Long-Range Network Management

The M5Stack Unit C6L ESP32-C6 supports simultaneous BLE, USB, and 2.4GHz Wi-Fi 6. You can configure the device with your home Wi-Fi credentials to place it anywhere in or around your house and manage it over your local network without needing to stay in Bluetooth range.

#### 1. Configuring Wi-Fi Credentials
Connect to the device via Bluetooth (using the random PIN displayed on screen) or via USB in the MeshCore Web App (https://app.meshcore.nz/) or mobile app:
1. Open the **Terminal / CLI** in the app.
2. Enter your home Wi-Fi network details:
   ```
   set wifi.ssid YourHomeNetwork
   set wifi.pwd YourHomePassword
   set wifi.enabled 1
   ```
3. The device immediately connects to your Wi-Fi router and requests an IP address via DHCP.

#### 2. The Dedicated Wi-Fi Screen (`HomePage::WIFI`)
Cycle through menu screens using single button clicks:
`FIRST` -> `RECENT` -> `RADIO` -> `BLUETOOTH` -> `WIFI` -> `ADVERT` -> `SHUTDOWN`

- **When Disconnected / Disabled**: Displays the 32x32 Wi-Fi icon (`wifi_on` or `wifi_off`) and status (`WIFI: OFF`, `No SSID`, or `Connecting`). Long-pressing the button toggles Wi-Fi ON / OFF and saves the preference to flash.
- **When Connected**: Displays:
  ```
  WiFi: Connected
     Home-WiFi
     192.168.
    1.105:5000
  ```
  The full IP address and TCP port (`5000`) are shown clearly across two lines with zero text clipping.

#### 3. Connecting to the Companion Over Wi-Fi
In the MeshCore mobile app or desktop tools:
- Select **Connect via TCP / Network**.
- Enter the device's local IP address (e.g. `192.168.1.105`) and Port `5000`.
- You can now manage contacts, send messages, and monitor packets from anywhere within your home Wi-Fi coverage!

---

### Changelog & Hardware Fixes

#### Latest Release Fixes
- **Wi-Fi Interface & Dedicated Toggle Screen (`HomePage::WIFI`)**:
  - Integrated `SerialWifiInterface` running on TCP port `5000`.
  - Added dedicated `WIFI` screen in the menu carousel with long-press toggle and real-time connection telemetry.
  - Dynamically connects upon CLI configuration without requiring a manual reboot.
  - Updated scrolling top status bar to include `WIFI:CONN`, `WIFI:ON`, or `WIFI:OFF`.
- **Partition Layout Scaled to `huge_app.csv`**:
  - To accommodate the complete BLE 5.3 stack, 2.4GHz Wi-Fi 6 subsystem, and OLED graphics simultaneously, the partition table was updated to `huge_app.csv`.
  - Allocates 3.0 MB for the application partition (`app0`, 66.7% used) and 896 KB for SPIFFS (`0xE0000`), providing ample storage for message logs and contacts.
- **Phantom GPS Screen Eliminated (`-UENV_INCLUDE_GPS`)**: The M5Stack Unit C6L has no onboard GPS hardware. Previously, `ENV_INCLUDE_GPS=1` was inherited from base configurations, creating a broken GPS page in the menu cycle that attempted to print both `"gps off"` and `"Can't access GPS"` at the identical coordinate `y=8`, causing the jumbled `Gps ' df 1 cces` artifact. Adding `-UENV_INCLUDE_GPS` and removing unused GPS pins completely removes this screen from the menu cycle.
- **Prominent Bluetooth PIN on First Screen**:
  - Replaced the unused battery voltage on the first (`FIRST`) screen with the random 6-digit Bluetooth PIN (`PIN:%06d` at `y=23`, centered).
  - Line 3 (`y=35`) now clearly indicates the connection state: `BLE Ready` (waiting for pairing), `Connected` (Bluetooth connected), `USB Link` (USB serial active), or `BLE: OFF` (Bluetooth disabled).
  - The PIN is always visible on the main screen upon boot, allowing quick and effortless pairing with mobile apps or MeshCore Web.
- **Battery Info Relocated to Radio Screen**: Since the Unit C6L is powered via external 5V (USB or Grove connector) and has no internal battery by default, `BAT: 0.00V` has been removed from the main screen and relocated to line 5 of the `RADIO` screen (`BAT:%.2fV` at `y=40`). It remains accessible for external power diagnostics without taking up valuable space on the main screen.
- **Accurate Bluetooth Connection Detection**: Fixed `ArduinoSerialInterface::isConnected()` under `ARDUINO_USB_CDC_ON_BOOT` and introduced `isBluetoothConnected()` to differentiate between USB serial link and active Bluetooth connections. Previously, being connected via USB caused `hasConnection()` to return true, which masked the Bluetooth PIN and falsely showed `"Connected"` on the Bluetooth screen even when BLE was still unpaired and awaiting connection. The PIN is now correctly displayed on both the main screen and the `BLUETOOTH` screen until an actual Bluetooth client connects.
- **Correct XBM Rendering (`drawXBitmap`)**: Switched `UnitC6LDisplay::drawXbm` to use `drawXBitmap` instead of `drawBitmap`. XBM files store pixel data LSB-first. Using MSB-first `drawBitmap` caused every 8-pixel byte to be mirrored horizontally, resulting in a backwards MeshCore splash logo, an inverted "fish-like" Bluetooth icon, inverted advert waves `)X(`, and an unrecognizable power button. All icons and splash logo are now crisp, upright, and non-mirrored.
- **Action Labels on Icon Screens**: Added intuitive text prompts directly below the 32x32 icons:
  - `BLUETOOTH`: Displays `PIN:%06d` when waiting for pairing, `Connected` when paired, or `BLE: OFF` when disabled.
  - `WIFI`: Displays `WIFI: OFF`, `No SSID`, or `Connecting` (or full IP when connected).
  - `ADVERT`: Displays `HOLD: SEND` below the broadcast antenna icon.
  - `SHUTDOWN`: Displays `HOLD: OFF` below the standard IEC Power icon, and updates to `Turning off...` while initiating shutdown.
- **Dynamic Random BLE PIN**: Removed static PIN override so the device generates a fresh 6-digit random PIN each session via `StdRNG` (between `100000` and `999999`), clearly shown on both the main screen (`FIRST`) and the `BLUETOOTH` screen for secure, straightforward pairing without hardcoded credentials.
- **OLED Text Wrap & Overwrite Fix**: Disabled text wrap (`display.setTextWrap(false)`) in `UnitC6LDisplay.h`. Adafruit_GFX defaults to `wrap = true`, which caused the 58-character scrolling status bar (`PL-test-m5stack-c6l | 0.00V | ...`) to wrap 6 times down the entire height of the 64x48 OLED every frame, overwriting all screens and tricking `getTextBounds` into thinking the text was static. Text now cleanly scrolls along line 0 (`y = 0..7`) without touching lower rows.
- **64x48 OLED UI Layout Optimization**: Adapted `ui-tiny/UITask.cpp` for the compact 64x48 screen:
  - `FIRST` page: Sized to `setTextSize(1)` so `MSG: %d`, `PIN:%06d`, and connection status fit within the 64-pixel width without clipping or vertical collisions.
  - `RADIO` page: Reorganized frequency, SF, bandwidth, CR, power, noise floor, and battery voltage into 5 clean 8px lines without horizontal character overlap.
  - `RECENT` page: Scaled vertical line pitch (`y += 9`) so up to 4 recent contact adverts fit cleanly between y=10 and y=44.
- **Splash Screen Isolation**: Excluded status bar rendering while `SplashScreen` is active (`curr == splash`), allowing the MeshCore logo (centered) and build date to display cleanly on boot.
- **Simultaneous USB + BLE Support**: Enabled `-D ENABLE_USB_INTERFACE` in the BLE environment. Both USB cable connection (Web Serial on https://app.meshcore.nz/) and Bluetooth work simultaneously.
- **Dual-Button Navigation & Wake**: Added `UnitC6LButton` to read both the front button (`P0` on PI4IO expander) and the side button (`GPIO 9` BOOT). Pressing either button wakes the screen from sleep and cycles menu pages.
- **OLED USB Power Retention**: Added `-D KEEP_DISPLAY_ON_USB=1` and overridden `isExternalPowered() = true`, preventing the 15-second display auto-off when powered over USB.
- **Clean Flash Tools**: Added `-Erase` switch to `flash-companion.ps1`, `flash-companion-clean.bat`, and `erase-flash.bat` to eliminate leftover flash/SPIFFS data from previous firmware.
- **SX1262 LDO Regulator Mode**: Enabled `-D SX126X_USE_REGULATOR_LDO=true` in `platformio.ini`. The Unit C6L (Stamp C6LoRa) routes SX1262 power through the internal LDO regulator rather than DC-DC; without this, `radio_init()` failed and halted the MCU in `while(1);` on boot.
- **SPI Bus CS Isolation**: Set LoRa NSS (`GPIO 23`) and OLED CS (`GPIO 6`) as `OUTPUT HIGH` before any SPI traffic begins. Prevents the SX1262 from receiving display initialization and drawing bytes over the shared SPI bus (`SCLK=20`, `MOSI=21`, `MISO=22`).
- **SX1262 Reset Pulse Timing**: Implemented clean 100ms active-low reset pulse on expander `P7` (`SX_NRST`) followed by release `HIGH` with LNA (`P5`) and antenna switch (`P6`) enabled.
- **OLED Display Orientation**: Added `display.setRotation(2)` (180° rotation) so that UI text and icons are right-side up when device antennas point upward.
- **USB CDC Configuration**: Added `-D ARDUINO_USB_CDC_ON_BOOT=1` and `-D ARDUINO_USB_MODE=1` to ensure reliable Web Serial and USB log monitoring.
- **Boot Fault Diagnostics**: Added visual fault feedback (RGB LED turns RED on `GPIO 2` and display outputs `"RADIO INIT / FAILED!"` if radio hardware initialization fails).

---

<a id="polski"></a>
## Polski

Rola oprogramowania dla **M5Stack Unit C6L**, łącząca się za pośrednictwem Bluetooth Low Energy (BLE), portu szeregowego USB oraz lokalnej sieci Wi-Fi 2.4 GHz (port TCP 5000) z aplikacjami mobilnymi (np. Aplikacja MeshCore / Web Flasher).

### Szczegóły Oprogramowania
- **Środowisko kompilacji**: `M5Stack_Unit_C6L_companion_radio_ble`
- **Rozmiar binariów**: ~2 120 KB
- **Użycie pamięci RAM**: 58.6% (192 KB / 328 KB)
- **Użycie pamięci Flash**: 66.7% (2 098 KB / 3 145 KB) w układzie partycji `huge_app.csv`
- **Kod PIN Bluetooth**: Losowy 6-cyfrowy kod generowany przy każdym uruchomieniu (wyświetlany na ekranie)
- **Serwer Wi-Fi TCP**: Aktywny na porcie TCP `5000`
- **Tryb pamięci Flash**: `DIO` (80 MHz)

### Pliki
- `firmware.bin` — Główny obraz aplikacji (adres pamięci flash `0x10000`)
- `bootloader.bin` — Bootloader ESP32-C6 (adres pamięci flash `0x0`)
- `partitions.bin` — Tabela partycji (adres pamięci flash `0x8000`)

### Instrukcja Wgrywania (Flashing)

#### Standardowe Wgrywanie (Zachowuje ustawienia)
Kliknij dwukrotnie `scripts\flash-companion.bat` lub uruchom:
```powershell
..\..\scripts\flash-companion.ps1
```

#### Czyste Wgrywanie (Zalecane — Czyści poprzedni SPIFFS i konfigurację)
Kliknij dwukrotnie `scripts\flash-companion-clean.bat` lub uruchom:
```powershell
..\..\scripts\flash-companion.ps1 -Erase
```

#### Pełne Czyszczenie Pamięci Flash (Erase Flash)
Kliknij dwukrotnie `scripts\erase-flash.bat` lub uruchom:
```powershell
..\..\scripts\erase-flash.ps1
```

Lub bezpośrednio za pomocą narzędzia `esptool.py`:
```bash
# 1. Pełne czyszczenie pamięci (zalecane)
python esptool.py --chip esp32c6 --baud 921600 erase_flash

# 2. Wgranie oprogramowania
python esptool.py --chip esp32c6 --baud 921600 write_flash 0x0 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
```

---

### Konfiguracja Wi-Fi i Zarządzanie w Sieci Domowej

Układ ESP32-C6 w module M5Stack Unit C6L obsługuje jednocześnie łączność Bluetooth LE, port szeregowy USB oraz sieć bezprzewodową Wi-Fi 6 (2.4 GHz). Umożliwia to skonfigurowanie danych sieci domowej i umieszczenie urządzenia w dowolnym miejscu domu lub ogrodu z dala od zasięgu Bluetooth, zachowując pełne zarządzanie przez sieć lokalną.

#### 1. Wprowadzanie Danych Sieci Wi-Fi
Połącz się z urządzeniem przez Bluetooth (korzystając z kodu PIN wyświetlanego na ekranie) lub kablem USB w aplikacji MeshCore (https://app.meshcore.nz/):
1. Otwórz zakładkę **Terminal / CLI** w aplikacji.
2. Wpisz parametry swojej sieci Wi-Fi:
   ```
   set wifi.ssid TwojaNazwaSieci
   set wifi.pwd TwojeHasloWiFi
   set wifi.enabled 1
   ```
3. Urządzenie natychmiast nawiąże połączenie z routerem i pobierze adres IP przez DHCP.

#### 2. Dedykowany Ekran Wi-Fi (`HomePage::WIFI`)
Przełączanie stron odbywa się pojedynczym kliknięciem przycisku:
`FIRST` -> `RECENT` -> `RADIO` -> `BLUETOOTH` -> `WIFI` -> `ADVERT` -> `SHUTDOWN`

- **Gdy Rozłączone / Wyłączone**: Wyświetla ikonę Wi-Fi 32x32 (`wifi_on` lub `wifi_off`) oraz stan (`WIFI: OFF`, `No SSID` lub `Connecting`). Przytrzymanie przycisku włącza lub wyłącza Wi-Fi i zapisuje stan w pamięci flash.
- **Gdy Połączone**: Wyświetla szczegóły połączenia:
  ```
  WiFi: Connected
     TwojaSiec
     192.168.
    1.105:5000
  ```
  Pełny adres IP oraz port TCP (`5000`) są wyświetlane w dwóch czytelnych liniach bez obcinania tekstu.

#### 3. Łączenie z Radiem przez Wi-Fi
W aplikacji mobilnej MeshCore lub w narzędziach komputerowych:
- Wybierz opcję **Connect via TCP / Network**.
- Wpisz lokalny adres IP urządzenia (np. `192.168.1.105`) oraz port `5000`.
- Możesz teraz wysyłać wiadomości, zarządzać kontaktami i monitorować sieć mesh z dowolnego miejsca w zasięgu domowej sieci Wi-Fi!

---

### Historia Zmian i Poprawki Sprzętowe

#### Najnowsze Poprawki
- **Obsługa Wi-Fi i Dedykowany Ekran Przełączania (`HomePage::WIFI`)**:
  - Zintegrowano interfejs `SerialWifiInterface` nasłuchujący na porcie TCP `5000`.
  - Dodano dedykowany ekran `WIFI` w karuzeli menu z możliwością włączania i wyłączania przez długie przytrzymanie przycisku oraz podglądem parametrów połączenia.
  - Automatyczne łączenie zaraz po skonfigurowaniu danych w CLI bez wymogu restartu urządzenia.
  - Zaktualizowano przewijany pasek statusu, aby pokazywał `WIFI:CONN`, `WIFI:ON` lub `WIFI:OFF`.
- **Tabela Partycji Rozszerzona do `huge_app.csv`**:
  - Aby pomieścić pełny stos Bluetooth 5.3, podsystem Wi-Fi 6 oraz biblioteki graficzne SSD1306, zmieniono tabelę partycji z `min_spiffs.csv` na `huge_app.csv`.
  - Przydzielono 3.0 MB na partycję aplikacji (`app0`, zajęte w 66.7%) oraz 896 KB na partycję SPIFFS (`0xE0000`), co zapewnia dużą przestrzeń na bazę kontaktów i wiadomości.
- **Usunięcie Nieprawidłowego Ekranu GPS (`-UENV_INCLUDE_GPS`)**: Moduł M5Stack Unit C6L nie posiada wbudowanego odbiornika GPS. Poprzednio flaga `ENV_INCLUDE_GPS=1` była automatycznie dziedziczona z konfiguracji bazowych, tworząc uszkodzony ekran GPS, na którym teksty `"gps off"` oraz `"Can't access GPS"` nakładały się na siebie w tej samej linii (`y=8`), tworząc nieczytelne znaki `Gps ' df 1 cces`. Dodanie `-UENV_INCLUDE_GPS` i usunięcie nieużywanych pinów całkowicie wyeliminowało ten ekran z menu urządzenia.
- **Wyświetlanie Losowego Kodu PIN Bluetooth na Pierwszym Ekranie**:
  - Usunięto nieużyteczny odczyt baterii z pierwszego ekranu (`FIRST`) i zastąpiono go losowym 6-cyfrowym kodem PIN Bluetooth (`PIN:%06d` na pozycji `y=23`, wyśrodkowany).
  - Linia 3 (`y=35`) informuje o dokładnym stanie połączenia: `BLE Ready` (oczekiwanie na sparowanie), `Connected` (połączono z klientem Bluetooth), `USB Link` (aktywne połączenie USB) lub `BLE: OFF` (Bluetooth wyłączony w menu).
  - Kod PIN jest natychmiast widoczny po uruchomieniu urządzenia, co pozwala na bezproblemowe parowanie w aplikacji mobilnej lub w przeglądarce.
- **Przeniesienie Informacji o Baterii na Ekran Radiowy**: Ponieważ Unit C6L jest zasilany napięciem zewnętrznym 5V (USB lub Grove) i nie posiada wbudowanego akumulatora, odczyt `BAT: 0.00V` został usunięty z ekranu głównego i przeniesiony do linii 5 na ekranie parametrów radiowych (`RADIO` -> `BAT:%.2fV` na pozycji `y=40`).
- **Precyzyjne Wykrywanie Połączenia Bluetooth**: Poprawiono implementację `ArduinoSerialInterface::isConnected()` pod `ARDUINO_USB_CDC_ON_BOOT` i dodano funkcję `isBluetoothConnected()`, aby odróżnić połączenie przewodowe USB od aktywnego połączenia BLE. Poprzednio podłączenie urządzenia do portu USB fałszywie sygnalizowało stan `"Connected"`, ukrywając kod PIN na ekranie Bluetooth przed faktycznym sparowaniem telefonu.
- **Poprawne Renderowanie Ikon XBM (`drawXBitmap`)**: Przełączono funkcję `UnitC6LDisplay::drawXbm` na `drawXBitmap` (odczyt LSB-first) zamiast `drawBitmap` (odczyt MSB-first). Format XBM przechowuje piksele w bajtach w odwrotnej kolejności bitów. Poprzednio każda 8-pikselowa kolumna była lustrzanie odwrócona, co dawało odwrócone logo powitalne MeshCore, zniekształconą ikonę Bluetooth przypominającą rybę, odwrócone fale nadawania advert `)X(` oraz nierozpoznawalny przycisk wyłączania. Wszystkie ikony i logo są teraz prawidłowo zorientowane, ostre i nieodwrócone.
- **Etykiety Akcji Pod Ikonami**: Dodano intuicyjne podpisy tekstowe bezpośrednio pod ikonami 32x32:
  - `BLUETOOTH`: Pokazuje `PIN:%06d` podczas oczekiwania na sparowanie, `Connected` po połączeniu lub `BLE: OFF` po wyłączeniu.
  - `WIFI`: Wyświetla `WIFI: OFF`, `No SSID` lub `Connecting` (lub pełny adres IP po połączeniu).
  - `ADVERT`: Wyświetla `HOLD: SEND` pod ikoną anteny (długie przytrzymanie wysyła pakiet rozgłoszeniowy advert).
  - `SHUTDOWN`: Wyświetla `HOLD: OFF` pod ikoną zasilania (długie przytrzymanie usypia / wyłącza urządzenie) oraz `Turning off...` podczas procedury wyłączania.
- **Dynamiczny Losowy Kod PIN BLE**: Usunięto statyczny PIN, dzięki czemu urządzenie generuje świeży, losowy 6-cyfrowy kod PIN (`100000`–`999999`) przy każdym uruchomieniu za pośrednictwem `StdRNG`, widoczny na ekranie głównym i ekranie Bluetooth.
- **Wyłączenie Zawijania Tekstu OLED (`setTextWrap(false)`)**: Wyłączono domyślne zawijanie tekstu biblioteki Adafruit_GFX, co zapobiega rozciąganiu 58-znakowego tekstu paska statusu na cały ekran w pionie i niszczeniu układu graficznego. Tekst płynnie przesuwa się w linii 0 (`y=0..7`).
- **Optymalizacja Układu UI dla Ekranu 64x48 OLED**: Dostosowano `ui-tiny/UITask.cpp` do małego ekranu 64x48:
  - Ekran `FIRST`: Rozmiar tekstu ustawiony na 1, dzięki czemu `MSG: %d`, `PIN:%06d` i status połączenia mieszczą się w szerokości 64 pikseli bez kolizji.
  - Ekran `RADIO`: Przeorganizowano parametry (częstotliwość, SF, pasmo, CR, moc nadawania, poziom szumów i napięcie) w 5 czytelnych linii o wysokości 8 pikseli.
  - Ekran `RECENT`: Zmniejszono odstępy w pionie (`y += 9`), dzięki czemu na ekranie mieszczą się do 4 ostatnio odebranych kontaktów.
- **Izolacja Ekranu Powitalnego**: Pasek statusu nie jest renderowany podczas wyświetlania ekranu powitalnego (`SplashScreen`), dzięki czemu logo MeshCore i data kompilacji są wyświetlane czysto i czytelnie na środku ekranu.
- **Jednoczesna Obsługa USB i BLE**: Włączono `-D ENABLE_USB_INTERFACE`, umożliwiając jednoczesne działanie połączenia przewodowego USB (MeshCore Web Serial) oraz transmisji bezprzewodowej Bluetooth.
- **Nawigacja Dwoma Przyciskami**: Zaimplementowano klasę `UnitC6LButton`, obsługującą zarówno przedni przycisk (`P0` na ekspanderze PI4IO), jak i boczny przycisk (`GPIO 9` BOOT). Naciśnięcie dowolnego przycisku wybudza ekran z uśpienia i przełącza strony w menu.
- **Podtrzymanie Ekranu na Zasilaniu USB**: Dodano flagę `-D KEEP_DISPLAY_ON_USB=1` oraz przeciążono `isExternalPowered() = true`, zapobiegając samoczynnemu wygaszaniu ekranu po 15 sekundach przy zasilaniu z portu USB.
- **Narzędzia Czystego Wgrywania**: Dodano przełącznik `-Erase` do `flash-companion.ps1` oraz pliki `flash-companion-clean.bat` i `erase-flash.bat`, pozwalające na wyczyszczenie pozostałości po starym oprogramowaniu.
- **Tryb Regulatora LDO SX1262**: Skonfigurowano `-D SX126X_USE_REGULATOR_LDO=true`, co odpowiada układowi zasilania Stamp C6LoRa i rozwiązuje problem zawieszania się radia przy starcie.
- **Izolacja Linii CS Magistrali SPI**: Zapewniono wymuszenie stanu wysokiego na liniach LoRa NSS (`GPIO 23`) i OLED CS (`GPIO 6`) przed rozpoczęciem komunikacji SPI, eliminując zakłócenia między radiem a wyświetlaczem.
- **Czasy Resetu SX1262**: Wdrożono czysty impuls resetujący (100 ms stan niski) na wyjściu `P7` ekspandera, a następnie stan wysoki z jednoczesnym włączeniem wzmacniacza LNA (`P5`) i przełącznika anteny (`P6`).
- **Orientacja Wyświetlacza OLED**: Dodano obrót o 180° (`display.setRotation(2)`), dzięki czemu interfejs jest prawidłowo zorientowany przy antenach skierowanych w górę.
- **Konfiguracja USB CDC**: Dodano `-D ARDUINO_USB_CDC_ON_BOOT=1` i `-D ARDUINO_USB_MODE=1` w celu stabilnego logowania i obsługi Web Serial.
- **Diagnostyka Błędów Startu**: W przypadku awarii inicjalizacji modułu radiowego dioda RGB świeci na czerwono (`GPIO 2`), a na ekranie pojawia się komunikat `"RADIO INIT / FAILED!"`.
