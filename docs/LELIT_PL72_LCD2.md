# Lelit William PL72 + Waveshare ESP32-S3-Touch-LCD-2

Port firmware Smart Grind-by-Weight na płytkę **Waveshare ESP32-S3-Touch-LCD-2**
(ST7789 SPI 240×320 + dotyk CST816D) dla młynka **Lelit William PL72**.
Logika dozowania pozostaje bez zmian w stosunku do upstream.

## Zasada działania

PL72 mieli tylko wtedy, gdy przycisk add/stop jest wciśnięty (maks. ok. 19 s na jedno przytrzymanie).
Przekaźnik SSR Pololu 5426 podłączony równolegle do styków tego przycisku działa jak „wirtualny palec”:
GPIO HIGH oznacza mielenie, GPIO LOW zatrzymanie. Odpowiada to dokładnie logice firmware
(stan wysoki na wyjściu = silnik pracuje).

## Środowiska PlatformIO

| Środowisko | Do czego służy |
|---|---|
| `waveshare-esp32s3-touch-lcd-2` | Docelowy firmware |
| `waveshare-esp32s3-touch-lcd-2-mock` | Symulowana waga, do testu ekranu i dotyku bez HX711 |

```bash
pio run -e waveshare-esp32s3-touch-lcd-2-mock -t upload   # test ekranu/dotyku
pio run -e waveshare-esp32s3-touch-lcd-2 -t upload        # docelowy firmware
pio device monitor -b 115200
```

## Piny (`src/config/hardware.h`)

| Funkcja | GPIO | Uwagi |
|---|---|---|
| LCD MOSI / SCLK / CS / DC | 38 / 39 / 45 / 42 | wg demo Waveshare |
| LCD podświetlenie | 1 | PWM (jasność w menu) |
| Dotyk SDA / SCL | 48 / 47 | CST816D, adres 0x15, wspólna magistrala z IMU |
| HX711 SCK | 15 | złącze P2, pin 7 (linia kamery, wolna bez kamery) |
| HX711 DOUT | 13 | złącze P2, pin 8 |
| SSR (Pololu 5426 EN) | 11 | złącze P2, pin 9 |
| 3V3 / GND | – | złącze P2, piny 1 / 2 (drugi GND: pin 13) |
| ADC baterii | 5 | dzielnik ×3 (wskaźnik baterii) |

**Wszystkie sygnały są na jednym złączu P2** (schemat Waveshare ESP32-S3-Touch-LCD-2_V2). Przed lutowaniem porównaj z opisem na płytce. Nie podłączaj kamery – używa tych samych GPIO. Jeśli wybierzesz inne,
zmień je w `hardware.h`. Unikaj pinów strapping 0, 3, 45, 46 i UART0 (43/44).

## Okablowanie

- **HX711:** VCC → 3V3, GND → GND, DT → GPIO13, SCK → GPIO15. Tensometr MAVIN (5 przewodów): E+ czerwony, E− czarny, A+ zielony, A− biały, przeźroczysty = ekran → GND tylko po stronie HX711 (na tensometrze niepodłączony).
- **Pololu 5426:** VIN → 3V3, GND → GND, EN → GPIO11. **Rezystor 10–100 kΩ między EN a GND** utrzymuje przekaźnik wyłączony podczas resetu i bootu. Wyjście (2 piny) → równolegle do styków przycisku add/stop.
- **Zasilanie:** 18650 → KCD1 → złącze MX1.25 baterii na płytce (ładowanie przez USB-C, układ ETA6098). **Sprawdź polaryzację złącza miernikiem.**

## Zabezpieczenia

1. `setup()` jako pierwszą operację ustawia wyjście przekaźnika na LOW, a `Grinder::init()` dodatkowo włącza pull-down.
2. Zewnętrzny pull-down na EN utrzymuje przekaźnik wyłączony w fazie boot ROM.
3. `HW_GRINDER_MAX_CONTINUOUS_ON_MS = 18000`: niezależny timer w sterowniku młynka wymusza LOW po 18 s ciągłej pracy (poniżej 19-sekundowego limitu PL72).
4. `GRIND_TIMEOUT_SEC = 30`: limit całej sesji dozowania (upstream).
5. Każda awaria ESP (reset, brak zasilania, wyłączenie KCD1) zwalnia „wirtualny przycisk”, więc młynek się zatrzymuje.

## Przed pierwszym podłączeniem do młynka

- Zmierz napięcie na stykach przycisku add/stop przy włączonym młynku. Pololu 5426 przyjmuje **maks. 100 V**. Jeśli przycisk okaże się połączony z siecią 230 V, nie podłączaj przekaźnika.
- Najpierw przetestuj wyjście na diodzie LED lub mierniku.
- W PL72 ustaw tryb manualny i uruchom autotune w menu (zmierzy opóźnienie silnika).
- Firmware dosypuje impulsami (50–300 ms). Jeśli PL72 nie reaguje na najkrótsze impulsy, zwiększ minimalny czas impulsu w `src/config/grind_control.h`.

## Do zrobienia

- Odcięcie przy niskim napięciu baterii (GPIO5, ok. 3,3 V) i wskaźnik baterii w UI.
- Dopracowanie layoutu UI po pierwszych testach na ekranie 240×320.

## Pasek stanu, bateria i gesty (LCD2)

- **Pasek u góry na każdym ekranie:** po lewej bieżąca waga, po prawej bateria (%), ostrzeżenie, Bluetooth.
- **Bateria:** napięcie z GPIO5 (dzielnik 200k/100k, ×3), 16 próbek bez min/max, filtr; procent z krzywej Li-ion z histerezą (przy ładowaniu tylko rośnie, na baterii tylko spada). Ładowanie z linii CHG_DET (GPIO18, stan wysoki = ładowanie) – ikona błyskawicy. Podczas ładowania napięcie na ogniwie jest zawyżone o ~0,1–0,2 V, więc % po odłączeniu ładowarki spada – to normalne.
- **Kontrola baterii:** Menu → System Info pokazuje `Battery:` (napięcie, %, CHG) i `Batt ADC:` (mV na pinie i przeliczone V). Porównaj z multimetrem na ogniwie; przy różnicy ustaw `HW_BATTERY_VOLTAGE_CAL` w `src/config/hardware.h` (= napięcie z multimetru / napięcie z ekranu). Pełne ładowanie (0,5 A, 2500 mAh) trwa ok. 5 h.
- **Auto-tara po starcie:** ok. 2,5 s po włączeniu, jednorazowo, w tle (bez blokowania ekranu). Tara przy mieleniu i w menu Scale bez zmian.
- **Grind Settings → gesty pionowe:** „Mode” przełącza Weight/Time, „Profile” przełącza Single/Double/Custom (w górę = następny, w dół = poprzedni). Włączenie jednej opcji wyłącza drugą. W trybie „Profile” profile są ułożone pionowo (przewijanie w górę/dół z animacją pionową), a gest poziomy przełącza tylko między profilami a MAIN MENU.
