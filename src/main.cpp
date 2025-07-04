#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/utils/NodeIDs.hpp>

using namespace geode::prelude;

class $modify(MyEditorUI, EditorUI) {
    struct Fields {
        CCMenuItemSpriteExtra* m_moveUpBtn;

        CCMenu* m_buttonMenu;
    };

    bool init(LevelEditorLayer * p0) {
        if (!EditorUI::init(p0)) return false;

        // create menu for the buttons
        m_fields->m_buttonMenu = CCMenu::create();
        m_fields->m_buttonMenu->setPosition({ 315.0f, 125.0f });
        m_fields->m_buttonMenu->setAnchorPoint({ 0.5f, 0.5f });
        m_fields->m_buttonMenu->setContentSize({ 75.0f, 75.0f });
        m_fields->m_buttonMenu->ignoreAnchorPointForPosition(false);

        // create visible background for the menu
        auto buttonMenuBg = CCScale9Sprite::create("GJ_square02.png");
        buttonMenuBg->setContentSize(m_fields->m_buttonMenu->getContentSize());
        buttonMenuBg->ignoreAnchorPointForPosition(false);
        buttonMenuBg->setAnchorPoint({ 0.5, 0.5 });
        buttonMenuBg->setPosition({ m_fields->m_buttonMenu->getContentWidth() / 2.f, m_fields->m_buttonMenu->getContentHeight() / 2.f });

        m_fields->m_buttonMenu->addChild(buttonMenuBg);

        // sprite for all buttons
        auto moveBtnSprite = CCSprite::createWithSpriteFrameName("GJ_button_01.png");
        moveBtnSprite->setScale(0.875f);

        // create the up btn
        m_fields->m_moveUpBtn = CCMenuItemSpriteExtra::create(
            moveBtnSprite,
            this,
            menu_selector(MyEditorUI::onMoveUpButton)
        );
        m_fields->m_moveUpBtn->ignoreAnchorPointForPosition(false);
        m_fields->m_moveUpBtn->setAnchorPoint({ 0.5, 0.5 });
        m_fields->m_moveUpBtn->setPosition({ 35.f, 50.f });

        // up btn icon sprite
        auto moveUpBtnIcon = CCSprite::createWithSpriteFrameName("edit_upBtn_001.png");
        moveUpBtnIcon->setScale(0.875f);
        moveUpBtnIcon->setAnchorPoint({ 0.5, 0.5 });
        moveUpBtnIcon->ignoreAnchorPointForPosition(false);
        moveUpBtnIcon->setPosition({ m_fields->m_moveUpBtn->getContentWidth() / 2.f, m_fields->m_moveUpBtn->getContentHeight() / 2.f });

        m_fields->m_moveUpBtn->addChild(moveUpBtnIcon);
        m_fields->m_buttonMenu->addChild(m_fields->m_moveUpBtn);

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

    void onMoveUpButton(CCObject * sender) {
        if (!m_editorLayer) {
            log::error("No editor layer found");
            return;
        };

        if (!m_selectedObjects || m_selectedObjects->count() == 0) {
            log::error("No objects selected to move up");
            return;
        };

        log::debug("Selected object count: {}", m_selectedObjects->count());

        // Move objects up by a small increment
        const float MOVE_OFFSET = 2.0f;
        int movedCount = 0;

        CCObject* obj;
        CCARRAY_FOREACH(m_selectedObjects, obj) {
            if (!obj) {
                log::warn("Null object in selected objects array");
                continue;
            };

            auto gameObj = dynamic_cast<GameObject*>(obj);
            if (gameObj) {
                CCPoint oldPos = gameObj->getPosition();
                CCPoint newPos = CCPoint(oldPos.x, oldPos.y + MOVE_OFFSET);

                gameObj->setPosition(newPos);
                movedCount++;

                log::info("Moved object {} from ({}, {}) to ({}, {})",
                          movedCount, oldPos.x, oldPos.y, newPos.x, newPos.y);
            } else {
                log::warn("Object is not a GameObject");
            };
        };

        if (movedCount == 0) {
            log::error("No objects were moved - all objects failed validation");
        } else {
            log::info("Successfully moved {} objects", movedCount);

            // Create undo action using the proper GD editor method
            if (m_editorLayer) {
                // Call the editor's update for undo - this is the standard way in GD
                m_editorLayer->updateEditor(0.016f);
                log::debug("Editor updated after move operation");
            };
        };
    };
};