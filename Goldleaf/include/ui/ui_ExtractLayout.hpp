#pragma once
#include <ui/ui_Includes.hpp>

namespace ui {

    class ExtractLayout : public pu::ui::Layout {
        private:
            pu::ui::elm::ProgressBar::Ref extract_p_bar;
            pu::ui::elm::TextBlock::Ref info_text;
            pu::ui::elm::TextBlock::Ref speed_text;
        public:
            ExtractLayout();
            PU_SMART_CTOR(ExtractLayout)

            void Extract(const std::string& archivePath, fs::Explorer *exp);
    };
}