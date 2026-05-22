#include <Geode/Geode.hpp>
#include <Geode/modify/ItemInfoPopup.hpp>
#include <Geode/modify/GJGarageLayer.hpp>

using namespace geode::prelude;

static std::vector<int> getFavs(UnlockType type) {
    auto key = fmt::format("favs-{}", (int)type);
    return Mod::get()->getSavedValue<std::vector<int>>(key, {});
}
static void setFavs(UnlockType type, std::vector<int> v) {
    auto key = fmt::format("favs-{}", (int)type);
    Mod::get()->setSavedValue(key, v);
}
static bool isFav(int id, UnlockType type) {
    auto v = getFavs(type);
    return std::find(v.begin(), v.end(), id) != v.end();
}
static void toggleFav(int id, UnlockType type) {
    auto v = getFavs(type);
    auto it = std::find(v.begin(), v.end(), id);
    if (it != v.end()) v.erase(it);
    else v.push_back(id);
    setFavs(type, v);
}
class $modify(MyItemInfoPopup, ItemInfoPopup) {
    void refreshHeart() {
        auto btn = this->getChildByID("fav-btn");
        if (!btn) return;
        auto item = static_cast<CCMenuItemSpriteExtra*>(btn);
        bool fav = isFav(m_itemID, m_unlockType);
        auto spr = CCSprite::createWithSpriteFrameName(
            fav ? "gj_heartOn_001.png" : "gj_heartOff_001.png"
        );
        spr->setScale(0.7f);
        item->setNormalImage(spr);
    }
    void onFav(CCObject*) {
        toggleFav(m_itemID, m_unlockType);
        refreshHeart();
    }
    bool init(int id, UnlockType type) {
        if (!ItemInfoPopup::init(id, type)) return false;
        // fave button my beloved
        bool fav = isFav(id, type);
        auto spr = CCSprite::createWithSpriteFrameName(
            fav ? "gj_heartOn_001.png" : "gj_heartOff_001.png"
        );
        spr->setScale(0.7f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this,
            menu_selector(MyItemInfoPopup::onFav));
        btn->setID("fav-btn");
        auto menu = CCMenu::create();
        menu->setPosition(ccp(407.5f, 247.0f));
        menu->addChild(btn);
        this->addChild(menu, 10);
        return true;
    }
};
static constexpr int kFavIconTypeInt = 99;
class $modify(MyGarageLayer, GJGarageLayer) {
    struct Fields {
        bool m_showingFavs = false; // are we on the favs page?
    };
    void onFavsTab(CCObject*) {
        m_fields->m_showingFavs = true;
        for (int i = 0; i < m_tabButtons->count(); i++) {
            auto tb = static_cast<CCMenuItemSpriteExtra*>(m_tabButtons->objectAtIndex(i));
            if (tb) tb->setEnabled(true);
        }
        setupFavsPage();
    }
    void setupFavsPage() {
        if (m_iconSelection) {
            m_iconSelection->removeFromParent();
            m_iconSelection = nullptr;
        }
        auto allItems = CCArray::create();
        for (int ut = 0; ut <= 12; ut++) {
            auto favIds = getFavs(static_cast<UnlockType>(ut));
            for (int id : favIds) {
                auto icon = GJItemIcon::createBrowserItem(
                    static_cast<UnlockType>(ut), id
                );
                if (icon) allItems->addObject(icon);
            }
        }
        if (allItems->count() == 0) {
            auto lbl = CCLabelBMFont::create("No favourites yet!\nTap the heart on any icon.", "bigFont.fnt");
            lbl->setScale(0.4f);
            lbl->setAlignment(kCCTextAlignmentCenter);
            lbl->setPosition(ccp(225, 155));
            lbl->setID("favs-placeholder");
            this->addChild(lbl, 10);
            return;
        }
        if (auto ph = this->getChildByID("favs-placeholder")) ph->removeFromParent();
        m_iconSelection = ListButtonBar::create(allItems, ccp(225.f, 168.f), 12, 4, 30.f, 30.f, 0.f, 24.f, 1);
        m_iconSelection->m_delegate = this;
        this->addChild(m_iconSelection, 5);
    }

    // Hook selectTab: when the user picks any normal tab, leave fav mode
    void selectTab(IconType type) {
        m_fields->m_showingFavs = false;
        // Remove placeholder label if it exists
        if (auto ph = this->getChildByID("favs-placeholder")) ph->removeFromParent();
        GJGarageLayer::selectTab(type);
    }
    bool init() {
        if (!GJGarageLayer::init()) return false;
        auto catMenu = this->getChildByID("category-menu");
        if (!catMenu) return true;
        auto spr = CCSprite::createWithSpriteFrameName("favs.png"_spr);
        if (!spr) {
            // fallback
            spr = CCSprite::createWithSpriteFrameName("gj_heartOn_001.png");
            spr->setScale(0.8f);
        }
        auto favsBtn = CCMenuItemSpriteExtra::create(spr, this,
            menu_selector(MyGarageLayer::onFavsTab));
        favsBtn->setID("favs-tab-btn");
        catMenu->addChild(favsBtn);
        if (auto menu = dynamic_cast<CCMenu*>(catMenu)) {
            menu->alignItemsHorizontallyWithPadding(4.f);
        }
        return true;
    }
};