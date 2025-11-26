# Исправление проблемы с PNG файлами

## 🐛 Проблема

При первом запуске игры возникала ошибка:
```
Failed to load image
Provided path: game/assets/weapons/bullet.png
Reason: PNG not supported: unknown PNG chunk type
```

## ✅ Решение

PNG файлы, созданные через base64-декодирование, были некорректными.

### Что было сделано:

1. **Удалены старые некорректные PNG файлы**
2. **Создан скрипт для генерации валидных PNG**
   - Файл: `game/assets/weapons/create_png.py`
   - Использует чистый Python без внешних зависимостей
   - Создает корректные PNG файлы с нуля

3. **Созданы новые ассеты:**
   - `pistol-idle.png` (32x32) - ✅ валидный PNG
   - `pistol-shoot.png` (128x32) - ✅ валидный PNG
   - `bullet.png` (16x16) - ✅ валидный PNG
   - `bullet-impact.png` (64x16) - ✅ валидный PNG

## 📊 Проверка файлов

```bash
$ file game/assets/weapons/*.png
game/assets/weapons/bullet-impact.png: PNG image data, 64 x 16, 8-bit/color RGBA
game/assets/weapons/bullet.png:        PNG image data, 16 x 16, 8-bit/color RGBA
game/assets/weapons/pistol-idle.png:   PNG image data, 32 x 32, 8-bit/color RGBA
game/assets/weapons/pistol-shoot.png:  PNG image data, 128 x 32, 8-bit/color RGBA
```

Все файлы теперь корректны!

## 🚀 Запуск

Игра теперь должна запускаться без ошибок:

```bash
./quick_start.sh
```

или

```bash
./scripts/play.sh
```

## 🎨 Визуальные элементы

Новые PNG файлы содержат:
- Простые пиксельные спрайты оружия (серый/коричневый)
- Желтые светящиеся пули
- Анимацию вспышки выстрела
- Эффект взрыва при попадании

## 📝 Примечание

Если в будущем вам нужно изменить спрайты:
1. Отредактируйте `game/assets/weapons/create_png.py`
2. Запустите: `python3 game/assets/weapons/create_png.py`
3. Пересоберите проект: `./scripts/build.sh --release --without-tests`

---

**Проблема решена! ✅**

