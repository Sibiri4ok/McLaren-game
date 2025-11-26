#!/bin/bash

# Quick Start Script для McLaren с системой оружия
# Автоматическая сборка и запуск игры

echo "╔═══════════════════════════════════════════════════════════╗"
echo "║         McLaren Engine - Weapon System Demo              ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo ""
echo "🎮 Управление:"
echo "  W/A/S/D - Движение персонажа"
echo "  ПРОБЕЛ  - Стрельба из пистолета"
echo ""
echo "📋 Новые возможности:"
echo "  ✅ Система оружия с анимацией"
echo "  ✅ Стрельба в 4 направлениях"
echo "  ✅ Физика снарядов с коллизиями"
echo "  ✅ Визуальные эффекты выстрела"
echo "  ✅ Эффект отдачи"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Проверка наличия build директории
if [ ! -d "build" ]; then
    echo "📦 Первый запуск - создание директории сборки..."
    mkdir -p build
fi

# Проверка, нужна ли пересборка
NEEDS_REBUILD=false

if [ ! -f "bin/main" ]; then
    echo "⚠️  Исполняемый файл не найден"
    NEEDS_REBUILD=true
else
    # Проверка изменений в исходниках
    if [ "engine/ecs/systems.cpp" -nt "bin/main" ] || \
       [ "engine/ecs/components.h" -nt "bin/main" ] || \
       [ "game/loops/game_loop.cpp" -nt "bin/main" ]; then
        echo "🔄 Обнаружены изменения в исходниках"
        NEEDS_REBUILD=true
    fi
fi

if [ "$NEEDS_REBUILD" = true ]; then
    echo "🔨 Сборка проекта..."
    echo ""
    ./scripts/build.sh --release --without-tests
    
    if [ $? -ne 0 ]; then
        echo ""
        echo "❌ Ошибка сборки!"
        exit 1
    fi
    echo ""
    echo "✅ Сборка завершена успешно!"
else
    echo "✅ Используется существующая сборка"
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "🚀 Запуск игры..."
echo ""
echo "💡 Подсказка: Стреляйте в направлении последнего движения!"
echo ""
sleep 1

# Запуск игры
./scripts/play.sh

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "📖 Дополнительная информация:"
echo "  - Полная документация: WEAPON_SYSTEM.md"
echo "  - Примеры расширения: WEAPON_EXAMPLES.md"
echo "  - Сводка изменений: CHANGES_SUMMARY.md"
echo ""
echo "Спасибо за использование McLaren Engine! 🏎️"
echo ""

