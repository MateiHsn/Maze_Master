#include <GameController.hpp>

GameController::GameController(GameModel * gameModel, HardwareController * hw) {
  model = gameModel;
  hardware = hw;
  renderers = nullptr;
  rendererCount = 0;
  lastUpdateTime = 0;
  highscoreScrollIndex = 0;
  settingsSelectedOption = 0;
  matrixBrightness = 8;
}

void GameController::init() {
  hardware->init();
  hardware->playStartupSound();
}

void GameController::addRenderer(IRenderer * renderer) {
  IRenderer ** newRenderers = new IRenderer*[rendererCount];
  for(uint8_t i = 0; i < rendererCount; i ++) {
    newRenderers[i] = renderers[i];
  }

  newRenderers[rendererCount] = renderer;
  if(renderers != nullptr) delete[] renderers;
  renderers = newRenderers;
  rendererCount ++;
  renderer->init();
}

void GameController::renderAll() {
  for(uint8_t i = 0; i < rendererCount; i ++){ 
    renderers[i]->render(*model);
  }
}

void GameController::update() {
  uint32_t currentTime = millis();
  if(currentTime - lastUpdateTime < updateInterval) return;
  lastUpdateTime = currentTime;

  hardware->update();

  GameState state = model->getState();

  switch(state) {
    case GameState::SPLASH_ART:
      updateSplashScreen();
    break;
    case GameState::MENU:
      updateMenu();
    break;
    case GameState::SETTINGS:
      updateSettings();
    break;
    case GameState::HIGH_SCORES:
      updateHighscores();
    break;
    case GameState::ABOUT:
      updateAbout();
    break;
    case GameState::PLAYING:
      updateGameplay();
    break;
    case GameState::PAUSED:
      updatePaused();
    break;
    case GameState::GAME_OVER:
      updateGameOver();
    break;
    case GameState::VICTORY:
      updateVictory();
    break;
  }

  renderAll();
}

void GameController::updateSplashScreen() {
  const InputState & input = hardware->getInput();
  if(input.buttonJustPressed) {
    model->setState(GameState::MENU);
    hardware->playMenuSelect();
  }
}

void GameController::updateMenu() {
  processMenuInput();
}

void GameController::updateHighscores() {
  const InputState & input = hardware->getInput();

  if(input.buttonJustPressed) {
    model->setState(GameState::MENU);
    hardware->playMenuSelect();
  }
}

void GameController::updateSettings() {

}

void GameController::updateAbout() {
  const InputState & input = hardware->getInput();

  if(input.buttonJustPressed) {
    model->setState(GameState::MENU);
    hardware->playMenuSelect();
  }
}

void GameController::updateGameplay() {
  processGameplayInput();
  handlePlayerMovement();
  checkGameEvents();
}

void GameController::updatePaused() {
  const InputState & input = hardware->getInput();
  if(input.buttonJustPressed) {
    model->setState(GameState::PLAYING);
    hardware->playMenuSelect();
  }
}

void GameController::updateGameOver() {
  const InputState & input = hardware->getInput();
  if(input.buttonJustPressed) {
    model->resetGame();
    hardware->playMenuSelect();
  }
}

void GameController::updateVictory() {
  const InputState & input = hardware->getInput();
  if(input.buttonJustPressed) {
    model->resetGame();
    hardware->playMenuSelect();
  }
}

void GameController::processMenuInput() {
  const InputState & input = hardware->getInput();
  static bool lastUp = false;
  static bool lastDown = false;

  if(input.upPressed && !lastUp){
    model->selectPreviousMenuOption();
    hardware->playMenuMove();
  }
  if(input.downPressed && !lastDown) {
    model->selectNextMenuOption();
    hardware->playMenuMove();
  }

  lastUp = input.upPressed;
  lastDown = input.downPressed;

  if(input.buttonJustPressed) {
    model->confirmMenuSelection();
    hardware->playMenuSelect();
  }
}

void GameController::processGameplayInput() {
  const InputState & input = hardware->getInput();
  if(input.buttonJustPressed) {
    model->setState(GameState::PAUSED);
    hardware->playMenuSelect();
  }
}

void GameController::handlePlayerMovement() {
  const InputState & input = hardware->getInput();
  if(!hardware->canMove()) return;

  int16_t deltaX = 0, deltaY = 0;

  bool shouldMove = false;
  static bool lastUp = false, lastDown = false, lastLeft = false, lastRight = false;

  if(input.upPressed && !lastUp) {
    deltaY = -1;
    shouldMove = true;
  } else if(input.downPressed && !lastDown) {
    deltaY = 1;
    shouldMove = true;
  } else if(input.leftPressed && !lastLeft) {
    deltaX = -1;
    shouldMove = true;
  } else if(input.rightPressed && !lastRight) {
    deltaX = 1;
    shouldMove = true;
  }

  lastUp = input.upPressed;
  lastDown = input.downPressed;
  lastLeft = input.leftPressed;
  lastRight = input.rightPressed;

  if(shouldMove && model->movePlayer(deltaX, deltaY)) {
    hardware->resetMoveCooldown();
  }
}

void GameController::checkGameEvents() {
  const Player & player = model->getPlayer();
  if(!player.isAlive) {
    hardware->playPlayerDeath();
    return;
  }

  if(model->isCurrentLevelCleared()) {
    if(model->checkExitAt(player.column, player.row)) {
      hardware->playLevelComplete();
      if(model->getState() == GameState::VICTORY) {
        hardware->playVictory();
      }
    }
  }
}