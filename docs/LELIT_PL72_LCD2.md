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
| HX711 DOUT | 16 | linia kamery, wolna gdy kamera nie jest podłączona |
| HX711 SCK | 15 | j.w. |
| SSR (Pololu 5426 EN) | 18 | |
| ADC baterii | 5 | dzielnik ×3 (na razie nieużywany w firmware) |

**Przed lutowaniem porównaj GPIO 15, 16 i 18 z opisem złączy na płytce.** Jeśli wybierzesz inne,
zmień je w `hardware.h`. Unikaj pinów strapping 0, 3, 45, 46 i UART0 (43/44).

## Okablowanie

- **HX711:** VCC → 3V3, GND → GND, DT → GPIO16, SCK → GPIO15. Tensometr: E+ czerwony, E− czarny, A− biały, A+ zielony (typowo, sprawdź w karcie tensometru).
- **Pololu 5426:** VIN → 3V3, GND → GND, EN → GPIO18. **Rezystor 10–100 kΩ między EN a GND** utrzymuje przekaźnik wyłączony podczas resetu i bootu. Wyjście (2 piny) → równolegle do styków przycisku add/stop.
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
