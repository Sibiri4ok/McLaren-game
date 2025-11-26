# Примеры использования системы оружия

## Добавление нового типа оружия

### 1. Создание автомата (быстрая стрельба)

```cpp
// В game_loop.cpp, в методе init()
engine::Weapon machineGun;
machineGun.fireRate = 0.1f;      // 10 выстрелов в секунду
machineGun.bulletSpeed = 25.f;   // Быстрые пули
machineGun.shootAnimationState = 2;
m_registry.emplace<engine::Weapon>(player, machineGun);

engine::WeaponDisplay weaponDisplay;
weaponDisplay.textureName = "game/assets/weapons/machinegun-idle.png";
weaponDisplay.size = {32.f, 32.f};
m_registry.emplace<engine::WeaponDisplay>(player, weaponDisplay);
```

### 2. Создание снайперской винтовки (медленная стрельба, быстрые пули)

```cpp
engine::Weapon sniperRifle;
sniperRifle.fireRate = 1.5f;     // Медленная перезарядка
sniperRifle.bulletSpeed = 40.f;  // Очень быстрые пули
sniperRifle.shootAnimationState = 3;
m_registry.emplace<engine::Weapon>(player, sniperRifle);
```

### 3. Создание дробовика (множественные снаряды)

Модифицируйте `weaponSystem` в `systems.cpp`:

```cpp
// Внутри weaponSystem, после создания основной пули
if (wantsToShoot && weapon.timeSinceLastShot >= weapon.fireRate) {
    // ... существующий код ...
    
    // Для дробовика - создаем несколько пуль с разбросом
    for (int i = -2; i <= 2; i++) {
        float spread = i * 0.15f; // Угол разброса
        sf::Vector2f spreadDir = shootDir;
        
        // Применяем поворот для разброса
        float angle = std::atan2(spreadDir.y, spreadDir.x) + spread;
        spreadDir.x = std::cos(angle);
        spreadDir.y = std::sin(angle);
        
        auto bullet = registry.create();
        sf::Vector2f bulletPos = pos.value + spreadDir * 0.5f;
        registry.emplace<Position>(bullet, bulletPos);
        registry.emplace<Velocity>(bullet, spreadDir);
        registry.emplace<Speed>(bullet, weapon.bulletSpeed * 0.8f);
        
        // ... остальной код создания пули ...
    }
}
```

## Добавление урона и здоровья

### 1. Добавить компоненты здоровья

В `components.h`:

```cpp
/**
 * @brief Component representing entity health.
 */
struct Health {
    float current = 100.f;
    float maximum = 100.f;
};

/**
 * @brief Component representing damage dealt by projectile.
 */
struct Damage {
    float amount = 10.f;
};
```

### 2. Система урона

В `systems.cpp`:

```cpp
void damageSystem(entt::registry &registry) {
    auto projectiles = registry.view<const Position, const Projectile, const Damage>();
    auto damageable = registry.view<const Position, Health>();
    
    std::vector<entt::entity> toDestroy;
    
    for (auto bullet : projectiles) {
        const auto &bulletPos = projectiles.get<Position>(bullet);
        const auto &damage = projectiles.get<Damage>(bullet);
        
        for (auto entity : damageable) {
            const auto &entityPos = damageable.get<Position>(entity);
            auto &health = damageable.get<Health>(entity);
            
            // Проверка дистанции для попадания
            sf::Vector2f diff = entityPos.value - bulletPos.value;
            float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
            
            if (distance < 0.5f) { // Радиус попадания
                health.current -= damage.amount;
                toDestroy.push_back(bullet);
                
                if (health.current <= 0) {
                    // Уничтожить entity или добавить тег Dead
                    registry.emplace<Dead>(entity);
                }
                break;
            }
        }
    }
    
    for (auto entity : toDestroy) {
        registry.destroy(entity);
    }
}
```

## Добавление эффектов попадания

### 1. Создание particle system для эффектов

В `components.h`:

```cpp
/**
 * @brief Component for particle effect.
 */
struct ParticleEffect {
    std::string textureName;
    float lifetime = 0.5f;
    float timeAlive = 0.f;
    int frameCount = 4;
    int currentFrame = 0;
};
```

### 2. Создание эффекта при попадании

```cpp
// При попадании пули в стену или entity
void createImpactEffect(entt::registry &registry, const sf::Vector2f &position) {
    auto effect = registry.create();
    registry.emplace<Position>(effect, position);
    registry.emplace<Velocity>(effect, sf::Vector2f{0.f, 0.f});
    
    Renderable render;
    render.textureName = "game/assets/weapons/bullet-impact.png";
    render.textureRect = sf::IntRect({0, 0}, {16, 16});
    render.targetSize = {16.f, 16.f};
    registry.emplace<Renderable>(effect, render);
    
    ParticleEffect particle;
    particle.textureName = "game/assets/weapons/bullet-impact.png";
    particle.lifetime = 0.3f;
    particle.frameCount = 4;
    registry.emplace<ParticleEffect>(effect, particle);
}
```

## Добавление звуковых эффектов

### 1. Подключение SFML Audio

Движок уже использует SFML, поэтому можно добавить звуки:

```cpp
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

// В game_loop.h
class GameLoop {
private:
    sf::SoundBuffer m_gunShotBuffer;
    sf::Sound m_gunShot;
};

// В game_loop.cpp, в init()
if (!m_gunShotBuffer.loadFromFile("game/assets/sounds/gunshot.wav")) {
    // Handle error
}
m_gunShot.setBuffer(m_gunShotBuffer);
m_gunShot.setVolume(50.f);
```

### 2. Воспроизведение звука при выстреле

В `weaponSystem`:

```cpp
if (wantsToShoot && weapon.timeSinceLastShot >= weapon.fireRate) {
    // ... создание пули ...
    
    // Воспроизвести звук (нужно передать sound object через систему)
    // playSound("gunshot");
}
```

## Добавление системы боеприпасов

### 1. Компонент боеприпасов

В `components.h`:

```cpp
/**
 * @brief Component for ammunition management.
 */
struct Ammunition {
    int current = 30;      // Текущие патроны в обойме
    int magazine = 30;     // Размер обоймы
    int reserve = 90;      // Патроны в запасе
    float reloadTime = 2.f; // Время перезарядки
    float reloading = 0.f;  // Текущее время перезарядки
    bool isReloading = false;
};
```

### 2. Модификация weaponSystem

```cpp
void weaponSystem(entt::registry &registry, const Input &input, float dt) {
    auto view = registry.view<Position, Weapon, Ammunition, Animation, PlayerControlled>();
    
    for (auto entity : view) {
        auto &weapon = view.get<Weapon>(entity);
        auto &ammo = view.get<Ammunition>(entity);
        
        // Обработка перезарядки
        if (ammo.isReloading) {
            ammo.reloading += dt;
            if (ammo.reloading >= ammo.reloadTime) {
                int needed = ammo.magazine - ammo.current;
                int toReload = std::min(needed, ammo.reserve);
                ammo.current += toReload;
                ammo.reserve -= toReload;
                ammo.isReloading = false;
                ammo.reloading = 0.f;
            }
            continue;
        }
        
        // Автоматическая перезарядка или по клавише R
        if (ammo.current == 0 || input.isKeyDown(sf::Keyboard::Key::R)) {
            if (ammo.reserve > 0 && !ammo.isReloading) {
                ammo.isReloading = true;
                ammo.reloading = 0.f;
            }
            continue;
        }
        
        weapon.timeSinceLastShot += dt;
        bool wantsToShoot = input.isKeyDown(sf::Keyboard::Key::Space);
        
        if (wantsToShoot && weapon.timeSinceLastShot >= weapon.fireRate && ammo.current > 0) {
            weapon.timeSinceLastShot = 0.f;
            ammo.current--; // Расход патрона
            
            // ... создание пули ...
        }
    }
}
```

## Добавление UI для отображения боеприпасов

### 1. Простой текстовый UI

В `game_loop.cpp`:

```cpp
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>

// В collectRenderData
void GameLoop::collectRenderData(engine::RenderFrame &frame, engine::Camera &camera) {
    // ... существующий код ...
    
    // Отображение боеприпасов
    auto playerView = m_registry.view<const Ammunition, const PlayerControlled>();
    for (auto entity : playerView) {
        const auto &ammo = playerView.get<Ammunition>(entity);
        
        // Создать текст с информацией о патронах
        // (Требуется расширение RenderFrame для поддержки текста)
        // Текст: "Ammo: 25/30 (90)"
    }
}
```

## Добавление разных типов снарядов

### 1. Взрывной снаряд

```cpp
struct ExplosiveProjectile {
    float explosionRadius = 2.f;
    float explosionDamage = 50.f;
};

// При уничтожении взрывного снаряда
void createExplosion(entt::registry &registry, const sf::Vector2f &position, 
                     float radius, float damage) {
    auto damageable = registry.view<const Position, Health>();
    
    for (auto entity : damageable) {
        const auto &entityPos = damageable.get<Position>(entity);
        auto &health = damageable.get<Health>(entity);
        
        sf::Vector2f diff = entityPos.value - position;
        float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        
        if (distance < radius) {
            float damageMultiplier = 1.f - (distance / radius);
            health.current -= damage * damageMultiplier;
        }
    }
    
    // Создать визуальный эффект взрыва
    createExplosionEffect(registry, position);
}
```

## Интеграция с AI врагов

### 1. Враги уклоняются от пуль

```cpp
void npcDodgeBulletsSystem(entt::registry &registry, float dt) {
    auto npcs = registry.view<Position, Velocity, ChasingPlayer>(
        entt::exclude<PlayerControlled>);
    auto bullets = registry.view<const Position, const Velocity, const Projectile>();
    
    for (auto npc : npcs) {
        auto &npcPos = npcs.get<Position>(npc);
        auto &npcVel = npcs.get<Velocity>(npc);
        
        for (auto bullet : bullets) {
            const auto &bulletPos = bullets.get<Position>(bullet);
            const auto &bulletVel = bullets.get<Velocity>(bullet);
            
            // Проверить, летит ли пуля в сторону NPC
            sf::Vector2f toNPC = npcPos.value - bulletPos.value;
            float distance = std::sqrt(toNPC.x * toNPC.x + toNPC.y * toNPC.y);
            
            if (distance < 3.f) {
                // Вычислить вектор уклонения
                sf::Vector2f perpendicular = {-bulletVel.value.y, bulletVel.value.x};
                npcVel.value += perpendicular * 0.5f;
                break;
            }
        }
    }
}
```

---

## Полезные советы

1. **Балансировка**: Начните с низкой скорострельности и постепенно увеличивайте
2. **Визуальная обратная связь**: Всегда добавляйте анимации и эффекты для действий
3. **Оптимизация**: Ограничивайте количество активных снарядов (например, max 100)
4. **Тестирование**: Проверяйте производительность с большим количеством снарядов
5. **Модульность**: Держите системы независимыми для легкого расширения

## Дополнительные ресурсы

- Основная документация: [WEAPON_SYSTEM.md](WEAPON_SYSTEM.md)
- ECS архитектура: [entt documentation](https://github.com/skypjack/entt)
- SFML графика: [SFML documentation](https://www.sfml-dev.org/documentation/)

