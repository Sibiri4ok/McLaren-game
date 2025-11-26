# Исправление проблемы с отображением персонажа

## 🐛 Проблема

После добавления системы оружия персонаж перестал отображаться на экране, хотя оружие и выстрелы работали корректно.

## 🔍 Причина

Анимация персонажа содержала состояние `state 2` (стрельба), которое использовало текстуру пистолета `pistol-shoot.png` размером 32x32 вместо текстуры волка 64x64.

Когда персонаж стрелял, `weaponSystem` переключал его анимацию на `state 2`, и система рендеринга пыталась отрисовать маленький спрайт пистолета вместо персонажа.

### Код с проблемой:

```cpp
// В game_loop.cpp
std::unordered_map<int, engine::AnimationClip> wolfClips = {
    {0, {"game/assets/critters/wolf/wolf-idle.png", 4, 0.15f, frameRect}},
    {1, {"game/assets/critters/wolf/wolf-run.png", 8, 0.08f, frameRect}},
    {2, {"game/assets/weapons/pistol-shoot.png", 4, 0.075f, sf::IntRect({0, 0}, {32, 32})}},  // ❌ Проблема
};

// В systems.cpp - weaponSystem
if (anim.clips.find(weapon.shootAnimationState) != anim.clips.end() &&
    anim.state != weapon.shootAnimationState) {
    anim.state = weapon.shootAnimationState;  // ❌ Переключает на state 2 (пистолет)
}
```

## ✅ Решение

Разделены анимации персонажа и визуальные эффекты оружия:

### 1. Убрана анимация стрельбы из персонажа

Персонаж теперь использует только свои собственные анимации:
- `state 0` - idle (волк стоит)
- `state 1` - run (волк бежит)

```cpp
// game_loop.cpp
std::unordered_map<int, engine::AnimationClip> wolfClips = {
    {0, {"game/assets/critters/wolf/wolf-idle.png", 4, 0.15f, frameRect}},
    {1, {"game/assets/critters/wolf/wolf-run.png", 8, 0.08f, frameRect}},
    // state 2 удален ✅
};
```

### 2. Упрощена логика стрельбы

```cpp
// systems.cpp - weaponSystem
// Handle shooting animation (visual effect only, no character animation change)
if (registry.all_of<IsShooting>(entity)) {
    auto &shooting = registry.get<IsShooting>(entity);
    shooting.animationTime += dt;

    // Remove shooting state after animation completes (just for weapon visual effect)
    if (shooting.animationTime >= 0.3f) {
        registry.remove<IsShooting>(entity);
    }
}
```

### 3. Удален неиспользуемый параметр

```cpp
// components.h - Weapon
struct Weapon {
    float fireRate = 0.5f;
    float timeSinceLastShot = 0.f;
    float bulletSpeed = 15.f;
    // shootAnimationState удален ✅
};
```

## 🎨 Визуальные эффекты стрельбы

Теперь эффект выстрела отображается **только через WeaponDisplay**:

1. **Персонаж** продолжает свою нормальную анимацию (idle/run)
2. **Оружие** (через `weaponDisplaySystem`) меняет спрайт на `pistol-shoot.png` с анимацией вспышки
3. **Пули** летят и отображаются корректно

### Как это работает:

```cpp
// weaponDisplaySystem проверяет компонент IsShooting
if (registry.all_of<IsShooting>(entity)) {
    weaponTexture = "game/assets/weapons/pistol-shoot.png";
    // Показывает анимацию вспышки выстрела
}
```

## 📊 Результат

✅ Персонаж (волк) отображается корректно  
✅ Оружие видно у персонажа  
✅ При стрельбе персонаж остается видимым  
✅ Вспышка выстрела отображается на оружии  
✅ Пули летят в правильном направлении  
✅ Все анимации работают плавно  

## 🎮 Что вы увидите в игре

```
     🌳                    🌳

  🪨    🐺🔫💥→→→         🌊
        (персонаж с оружием)
              
        🐺                🐺
         
  🌊                    🪨
     
     🌳                    🌳
```

- **Волк** - полностью видимый персонаж с нормальными анимациями
- **Пистолет** - маленький спрайт рядом с волком
- **Вспышка** - появляется на пистолете при выстреле
- **Пули** - желтые светящиеся точки

## 🚀 Запуск

Проект пересобран и готов к запуску:

```bash
./quick_start.sh
```

или

```bash
./scripts/play.sh
```

## 💡 Важное примечание

Теперь архитектура более логична:
- **Компонент Animation** - только для анимаций самого персонажа
- **Компонент WeaponDisplay** - для визуальных эффектов оружия
- **Компонент IsShooting** - временный тег для синхронизации визуальных эффектов

Эта архитектура позволяет:
- Легко добавлять новое оружие без изменения анимаций персонажа
- Персонажи с разными моделями могут использовать одно и то же оружие
- Визуальные эффекты оружия независимы от персонажа

---

**Проблема полностью решена! ✅**

Персонаж и оружие теперь отображаются корректно, все функции стрельбы работают как задумано.

