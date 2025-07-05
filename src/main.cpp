#include "./QuickMove.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/utils/NodeIDs.hpp>

using namespace geode::prelude;
using namespace quickmove;

// Get the move offset float depending on the set move distance
float getMoveOffset(MoveSize moveSize) {
    switch (moveSize) {
    case MoveSize::Tiny:
        return 0.5f;

    case MoveSize::Small:
        return 2.f;

    case MoveSize::Half:
        return 5.f;

    case MoveSize::Normal:
        return 10.f;

    case MoveSize::Big:
        return 25.f;

    default:
        return 2.f;
    };

    return 2.f;
};

// Returns the EditCommand based on object move size and direction
EditCommand getEditCmd(MoveSize moveSize, MoveDirection moveDir) {
    switch (moveSize) {
    case MoveSize::Tiny:
        switch (moveDir) {
        case MoveDirection::Up:    return EditCommand::TinyUp;
        case MoveDirection::Down:  return EditCommand::TinyDown;
        case MoveDirection::Left:  return EditCommand::TinyLeft;
        case MoveDirection::Right: return EditCommand::TinyRight;
        };
        break;

    case MoveSize::Small:
        switch (moveDir) {
        case MoveDirection::Up:    return EditCommand::SmallUp;
        case MoveDirection::Down:  return EditCommand::SmallDown;
        case MoveDirection::Left:  return EditCommand::SmallLeft;
        case MoveDirection::Right: return EditCommand::SmallRight;
        };
        break;

    case MoveSize::Half:
        switch (moveDir) {
        case MoveDirection::Up:    return EditCommand::HalfUp;
        case MoveDirection::Down:  return EditCommand::HalfDown;
        case MoveDirection::Left:  return EditCommand::HalfLeft;
        case MoveDirection::Right: return EditCommand::HalfRight;
        };
        break;

    case MoveSize::Normal:
        switch (moveDir) {
        case MoveDirection::Up:    return EditCommand::Up;
        case MoveDirection::Down:  return EditCommand::Down;
        case MoveDirection::Left:  return EditCommand::Left;
        case MoveDirection::Right: return EditCommand::Right;
        };
        break;

    case MoveSize::Big:
        switch (moveDir) {
        case MoveDirection::Up:    return EditCommand::BigUp;
        case MoveDirection::Down:  return EditCommand::BigDown;
        case MoveDirection::Left:  return EditCommand::BigLeft;
        case MoveDirection::Right: return EditCommand::BigRight;
        };
        break;

    default:
        return EditCommand::SmallUp;
    };

    return EditCommand::SmallUp;
};

class $modify(MyEditorUI, EditorUI) {
    struct Fields {
        MoveSize moveSize = MoveSize::Small;

        CCMenu* m_buttonMenu;

        CCMenuItemSpriteExtra* m_moveUpsmallBtn;
        CCMenuItemSpriteExtra* m_moveDownBtn;
        CCMenuItemSpriteExtra* m_moveLeftBtn;
        CCMenuItemSpriteExtra* m_moveRightBtn;
    };

    bool init(LevelEditorLayer * p0) {
        if (!EditorUI::init(p0)) return false;

        // create menu for the buttons
        m_fields->m_buttonMenu = CCMenu::create();
        m_fields->m_buttonMenu->setID("quick-move-menu"_spr);
        m_fields->m_buttonMenu->setPosition({ 315.0f, 125.0f });
        m_fields->m_buttonMenu->setAnchorPoint({ 0.5f, 0.5f });
        m_fields->m_buttonMenu->setContentSize({ 75.0f, 75.0f });
        m_fields->m_buttonMenu->ignoreAnchorPointForPosition(false);

        // create visible background for the menu
        auto buttonMenuBg = CCScale9Sprite::create("square02_001.png");
        buttonMenuBg->setContentSize(m_fields->m_buttonMenu->getContentSize());
        buttonMenuBg->ignoreAnchorPointForPosition(false);
        buttonMenuBg->setAnchorPoint({ 0.5, 0.5 });
        buttonMenuBg->setPosition({ m_fields->m_buttonMenu->getContentWidth() / 2.f, m_fields->m_buttonMenu->getContentHeight() / 2.f });

        m_fields->m_buttonMenu->addChild(buttonMenuBg);

        m_fields->m_moveUpsmallBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_button_01.png"),
            this,
            menu_selector(MyEditorUI::onMoveUpButton)
        );
        m_fields->m_moveUpsmallBtn->setID("move-up-small");
        m_fields->m_moveUpsmallBtn->ignoreAnchorPointForPosition(false);
        m_fields->m_moveUpsmallBtn->setAnchorPoint({ 0.5, 0.5 });
        m_fields->m_moveUpsmallBtn->setPosition({ 35.f, 45.f });

        // up btn icon sprite
        auto moveUpBtnIcon = CCSprite::createWithSpriteFrameName("edit_upBtn_001.png");
        moveUpBtnIcon->setScale(0.875f);
        moveUpBtnIcon->setAnchorPoint({ 0.5, 0.5 });
        moveUpBtnIcon->ignoreAnchorPointForPosition(false);
        moveUpBtnIcon->setPosition({ m_fields->m_moveUpsmallBtn->getContentWidth() / 2.f, m_fields->m_moveUpsmallBtn->getContentHeight() / 2.f });

        m_fields->m_moveUpsmallBtn->addChild(moveUpBtnIcon);
        m_fields->m_buttonMenu->addChild(m_fields->m_moveUpsmallBtn);

        // add the whole menu
        this->addChild(m_fields->m_buttonMenu);

        return true;
    };

    void showUI(bool show) {
        EditorUI::showUI(show);

        if (m_fields->m_buttonMenu) {
            m_fields->m_buttonMenu->setVisible(show);
        };
    };

    // Move all objects using the built-in move functionality
    void onMove(EditCommand editCommand = EditCommand::SmallUp, float MOVE_OFFSET = 2.f) {
        if (!m_editorLayer) {
            log::error("No editor layer found");
            return;
        };

        // Check if there are objects selected
        if ((!m_selectedObjects || m_selectedObjects->count() == 0) && !m_selectedObject) {
            log::error("No objects selected to move");
            return;
        };

        log::info("Using built-in move command: {}", static_cast<int>(editCommand));

        // did you know that this function was the only thing i needed, wasted 4 hours of my time adding CCPoint :D
        this->moveObjectCall(editCommand);

        log::info("Successfully executed move command with undo support");
    };

    void onMoveUpButton(CCObject*) {
        this->onMove(getEditCmd(m_fields->moveSize, MoveDirection::Up), getMoveOffset(m_fields->moveSize));
    };
};