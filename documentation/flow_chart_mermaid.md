# Flow Chart - Rapid Texter

Dokumentasi flow chart aplikasi Rapid Texter dipecah menjadi 3 bagian:

## Daftar Flow Chart

| File | Deskripsi |
|------|-----------|
| [flow_menu.md](flow_menu.md) | Navigasi menu: Main, History, Language, Duration, Mode, Difficulty |
| [flow_gameplay.md](flow_gameplay.md) | Game loop, input handling, dan tampilan hasil |
| [flow_credits.md](flow_credits.md) | Credits screen dan Rick Roll easter egg |

## Koneksi Antar Flow

Flow chart menggunakan **node lingkaran** untuk menunjukkan referensi ke flow lain:

- `((PLAY))` - Entry point ke flow gameplay
- `((MENU_DIFF))` - Kembali ke menu difficulty
- `((MENU_MODE))` - Kembali ke menu mode
- `((CREDITS))` - Entry point ke flow credits
- `((RICKROLL))` - Entry point easter egg (dari Hard completion)
- `((HISTORY))` - Entry point ke menu history

## Legend Simbol

| Simbol | Bentuk | Keterangan |
|--------|--------|------------|
| `([...])` | Stadium | Start/End node |
| `[...]` | Kotak | Process/Action |
| `{...}` | Belah Ketupat | Decision/Kondisi |
| `[/...)/]` | Jajar Genjang | Input/Output |
| `((...))` | Lingkaran | Referensi ke Flow Lain |

## Warna

- **Hijau** - Start/End
- **Biru** - Process
- **Oranye** - Decision
- **Ungu** - Input/Output
- **Pink** - Flow Reference
