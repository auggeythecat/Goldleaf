#include <ui/ui_ExtractLayout.hpp>
#include <ui/ui_MainApplication.hpp>
#include <fs/fs_Archive.hpp>

extern ui::MainApplication::Ref g_MainApplication;
extern cfg::Settings g_Settings;

namespace ui {
    
    ExtractLayout::ExtractLayout() : pu::ui::Layout() {
        const s32 p_bar_x = 120;
        this->extract_p_bar = pu::ui::elm::ProgressBar::New(p_bar_x, 800, pu::ui::render::ScreenWidth - (2 * p_bar_x), 30, 100.0f);
        this->extract_p_bar->SetVisible(false);
        this->extract_p_bar->SetProgressColor(g_Settings.GetColorScheme().progress_bar);
        this->extract_p_bar->SetBackgroundColor(g_Settings.GetColorScheme().progress_bar_bg);
        
        this->info_text = pu::ui::elm::TextBlock::New(0, 290, "A");
        this->info_text->SetHorizontalAlign(pu::ui::elm::HorizontalAlign::Center);
        this->info_text->SetColor(g_Settings.GetColorScheme().text);
        
        this->speed_text = pu::ui::elm::TextBlock::New(0, 350, "B");
        this->speed_text->SetHorizontalAlign(pu::ui::elm::HorizontalAlign::Center);
        this->speed_text->SetColor(g_Settings.GetColorScheme().text);
        
        this->Add(this->extract_p_bar);
        this->Add(this->info_text);
        this->Add(this->speed_text);
    }

    void ExtractLayout::Extract(const std::string& archivePath, fs::Explorer *exp) {
        // TODO: Added strings to config files
        auto last_tp = std::chrono::steady_clock::now();
        this->extract_p_bar->SetVisible(true);
        bool success = fs::Archive::ExtractArchive(archivePath, [&](const u64 total_size) {
            this->extract_p_bar->SetMaxProgress(total_size);
            this->info_text->SetText("Extracting: " + archivePath);
        }, [&](const u64 bytes_proccessed) {
            const auto cur_tp = std::chrono::steady_clock::now();
            const auto time_diff = (double)std::chrono::duration_cast<std::chrono::milliseconds>(cur_tp - last_tp).count();
            last_tp = cur_tp;

            const auto speed_bps = (1000.0f / time_diff) * bytes_proccessed;
            const auto speed_text =  cfg::Strings.GetString(458) + ": " + fs::FormatSize(speed_bps) + "/s, " + cfg::Strings.GetString(459) + ": " + util::FormatTime((u64)((1.0f / speed_bps) * (this->extract_p_bar->GetMaxProgress() - this->extract_p_bar->GetProgress())));
            this->speed_text->SetText(speed_text);

            this->extract_p_bar->IncrementProgress(bytes_proccessed);
            g_MainApplication->CallForRender();
        });

        this->extract_p_bar->SetVisible(false);
        if (success) {
            g_MainApplication->ShowNotification("Archive extracted successfully.");
        } else {
            g_MainApplication->ShowNotification("Failed to extract archive.");
        }
    }
}