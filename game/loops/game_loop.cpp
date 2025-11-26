#include "game_loop.h"

#include "core/camera.h"
#include "core/engine.h"
#include "core/render.h"
#include "core/render_frame.h"
#include "ecs/components.h"
#include "ecs/systems.h"
#include "ecs/utils.h"
#include "ecs/world_loader.h"
#include "resources/image_manager.h"
#include <random>

GameLoop::GameLoop() {
	engine::WorldLoader::loadWorldFromJson("game/assets/worlds/meadow.json", width,
										   height, tileTextures, tiles);
}

void GameLoop::init() {
	m_engine = engine::Engine::get();
	sf::Vector2f worldCenter = {width / 2.0f, height / 2.0f};
	sf::Vector2f screenCenter = m_engine->camera.worldToScreen(worldCenter);
	m_engine->camera.position = screenCenter;

	// Create and generate world tiles once

	const int tileWidth = 32.f;
	const int tileHeight = 32.f;
	m_engine->camera.setTileSize(tileWidth, tileHeight / 2);

	auto tileImages = engine::makeTileData(tileTextures, m_engine->imageManager);

	std::vector<engine::Tile> staticTiles = tiles;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			const auto &tile = tiles[y * width + x];
			std::vector<int> groundLayers;

			for (int key : tile.layerIds) {
				const auto &texInfo = tileTextures.at(key);

				if (texInfo.is_ground) {
					groundLayers.push_back(key);
				} else {
					sf::Vector2f worldPos = {(float)x + 2.f, (float)y + 1.f};

					auto stObject = systems::createStaticObject(
						m_registry, worldPos, {32.f, 32.f}, texInfo.texture_src,
						sf::IntRect({0, 0}, {32, 32}));
					m_registry.emplace<engine::CastsShadow>(stObject);
				}
			}
			staticTiles[y * width + x].layerIds = std::move(groundLayers);
		}
	}

	m_engine->render.generateTileMapVertices(m_staticMapPoints, m_engine->camera,
											 staticTiles, width, height, tileImages);

	// Create entities (player, NPC, etc.)

	sf::Vector2f targetWolfSize{64.f, 64.f};
	sf::IntRect frameRect({0, 0}, {64, 64});

	// Wolf
	std::unordered_map<int, engine::AnimationClip> wolfClips = {
		{0, {"game/assets/critters/wolf/wolf-idle.png", 4, 0.15f, frameRect}},
		{1, {"game/assets/critters/wolf/wolf-run.png", 8, 0.08f, frameRect}},
	};

	// Player wolf
	auto wolf =
		systems::createNPC(m_registry, {5.f, 5.f}, targetWolfSize, wolfClips, 5.f);
	m_registry.emplace<engine::PlayerControlled>(wolf);
	m_registry.emplace<engine::CastsShadow>(wolf);
	
	// Add health to player
	engine::Health playerHealth;
	playerHealth.current = 100.f;
	playerHealth.maximum = 100.f;
	m_registry.emplace<engine::Health>(wolf, playerHealth);
	
	// Add weapon to player
	engine::Weapon weapon;
	weapon.fireRate = 0.3f;
	weapon.bulletSpeed = 20.f;
	m_registry.emplace<engine::Weapon>(wolf, weapon);
	
	// Add weapon display to player
	engine::WeaponDisplay weaponDisplay;
	weaponDisplay.textureName = "game/assets/weapons/pistol-idle.png";
	weaponDisplay.offset = {0.3f, -0.1f};
	weaponDisplay.size = {24.f, 24.f};
	m_registry.emplace<engine::WeaponDisplay>(wolf, weaponDisplay);

	// Enemy wolf 1 (chaser)
	auto wolf1 =
		systems::createNPC(m_registry, {8.f, 8.f}, targetWolfSize, wolfClips, 2.5f);
	m_registry.emplace<engine::ChasingPlayer>(wolf1);
	m_registry.emplace<engine::CastsShadow>(wolf1);
	
	engine::Health enemy1Health;
	enemy1Health.current = 100.f;
	enemy1Health.maximum = 100.f;
	m_registry.emplace<engine::Health>(wolf1, enemy1Health);
	
	engine::Weapon enemy1Weapon;
	enemy1Weapon.fireRate = 0.4f;
	enemy1Weapon.bulletSpeed = 18.f;
	m_registry.emplace<engine::Weapon>(wolf1, enemy1Weapon);
	
	engine::WeaponDisplay enemy1WeaponDisplay;
	enemy1WeaponDisplay.textureName = "game/assets/weapons/pistol-idle.png";
	enemy1WeaponDisplay.size = {24.f, 24.f};
	m_registry.emplace<engine::WeaponDisplay>(wolf1, enemy1WeaponDisplay);
	
	engine::AICombat enemy1AI;
	enemy1AI.shootInterval = 1.2f;
	enemy1AI.detectionRange = 12.f;
	enemy1AI.shootingRange = 9.f;
	m_registry.emplace<engine::AICombat>(wolf1, enemy1AI);

	// Wandering wolves (enemies)
	for (int i = 0; i < 2; i++) {
		auto npc = systems::createNPC(m_registry, {i + 10.f, 0.f}, targetWolfSize,
									  wolfClips, 1.f);
		m_registry.emplace<engine::CastsShadow>(npc);
		
		engine::Health npcHealth;
		npcHealth.current = 100.f;
		npcHealth.maximum = 100.f;
		m_registry.emplace<engine::Health>(npc, npcHealth);
		
		engine::Weapon npcWeapon;
		npcWeapon.fireRate = 0.5f;
		npcWeapon.bulletSpeed = 16.f;
		m_registry.emplace<engine::Weapon>(npc, npcWeapon);
		
		engine::WeaponDisplay npcWeaponDisplay;
		npcWeaponDisplay.textureName = "game/assets/weapons/pistol-idle.png";
		npcWeaponDisplay.size = {24.f, 24.f};
		m_registry.emplace<engine::WeaponDisplay>(npc, npcWeaponDisplay);
		
		engine::AICombat npcAI;
		npcAI.shootInterval = 1.5f + i * 0.3f;
		npcAI.detectionRange = 10.f;
		npcAI.shootingRange = 8.f;
		m_registry.emplace<engine::AICombat>(npc, npcAI);
	}
}

void GameLoop::gameAnimationSystem(float dt) {
	auto view =
		m_registry.view<engine::Animation, engine::Velocity, engine::Renderable>();

	for (auto entity : view) {
		auto &anim = view.get<engine::Animation>(entity);
		auto &vel = view.get<engine::Velocity>(entity);
		auto &render = view.get<engine::Renderable>(entity);

		int newState =
			(std::sqrt(vel.value.x * vel.value.x + vel.value.y * vel.value.y) > 0.1f)
				? 1
				: 0;

		if (anim.clips.find(newState) != anim.clips.end() &&
			anim.state != newState) {
			anim.state = newState;
			anim.frameIdx = 0;
			anim.frameTime = 0.f;
		}
	}
}

void GameLoop::update(engine::Input &input, float dt) {
	// Input and AI
	systems::playerInputSystem(m_registry, input);
	systems::aiCombatSystem(m_registry, input, dt);
	
	// Combat
	systems::weaponSystem(m_registry, input, dt);
	systems::projectileSystem(m_registry, tiles, width, height, dt);
	systems::damageSystem(m_registry);
	systems::deathSystem(m_registry);
	
	// Movement
	systems::npcFollowPlayerSystem(m_registry, dt);
	systems::npcWanderSystem(m_registry, dt);
	systems::movementSystem(m_registry, tiles, width, height, dt);
	
	// Animation
	systems::animationSystem(m_registry, dt);
	gameAnimationSystem(dt);

	// Camera follow
	auto playerView =
		m_registry.view<const engine::Position, const engine::PlayerControlled>();
	for (auto entity : playerView) {
		const auto &pos = playerView.get<const engine::Position>(entity);
		m_engine->camera.position = m_engine->camera.worldToScreen(pos.value);
	}
}

void GameLoop::collectRenderData(engine::RenderFrame &frame,
								 engine::Camera &camera) {
	// Initialize vertex arrays
	frame.healthBarVertices.setPrimitiveType(sf::PrimitiveType::Points);
	frame.healthBarVertices.clear();
	
	// Collecting static map texture
	frame.tileVertices = m_staticMapPoints;

	// Collecting entities
	systems::renderSystem(m_registry, frame, camera, m_engine->imageManager);
	
	// Render weapons on top of entities
	systems::weaponDisplaySystem(m_registry, frame, camera, m_engine->imageManager);
	
	// Render health bars on top of everything
	systems::healthBarSystem(m_registry, frame, camera);
}

bool GameLoop::isFinished() const { return m_finished; }
