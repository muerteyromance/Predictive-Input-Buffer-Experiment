#include "Console.hpp"

#include "State.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>
#include <cmath>
#include <string>

#ifdef GEODE_IS_WINDOWS
#include <Windows.h>
#endif

using namespace geode::prelude;

namespace buf {
    // Text is ASCII only on purpose: the Windows console code page would garble accents.

    static std::string ticksLine(int baseTicks, int pct, bool release) {
        auto& st = state();
        double step = st.stepMs();
        double ms = baseTicks * step * pct / 100.0;
        if (release) ms *= st.cfg.releasePct / 100.0;
        ms = std::min(ms, static_cast<double>(st.cfg.maxMs));
        return fmt::format("{:.3f} ms -> {} ticks", ms, static_cast<int>(std::lround(ms / step)));
    }

    static std::string buildTitle() {
        auto const& st = state();
        auto const& pr = kPresets[st.preset];
        if (!pr.enabled) return "Predictive Input Buffer - OFF";
        if (st.holdDisabled) return "Predictive Input Buffer - PAUSADO";
        return fmt::format("Predictive Input Buffer - {} - {}/{} ticks", pr.name, pr.allTicks, pr.waveTicks);
    }

    static std::string buildPanel() {
        auto& st = state();
        auto const& cfg = st.cfg;
        auto const& pr = kPresets[st.preset];

        char const* status = !pr.enabled ? "OFF" : (st.holdDisabled ? "PAUSADO" : "ACTIVO");
        size_t waiting = st.queues[0].size() + st.queues[1].size();

        std::string t;
        t += fmt::format("Predictive Input Buffer | motor propio | {}\n", status);
        t += fmt::format("Preset: {} | {}\n", pr.name, pr.enabled ? "habilitado" : "deshabilitado");
        t += fmt::format("Paso fisico: {:.4f} ms ({} TPS, ajuste manual) | dt medido: {:.3f} ms\n",
            st.stepMs(), cfg.tps, st.c.lastDtMs);
        t += fmt::format("Click {} | Release {}\n",
            ticksLine(pr.allTicks, cfg.cubePct, false), ticksLine(pr.allTicks, cfg.cubePct, true));
        t += fmt::format("Wave click {} | Wave release {}\n",
            ticksLine(pr.waveTicks, cfg.wavePct, false), ticksLine(pr.waveTicks, cfg.wavePct, true));
        t += fmt::format("Nave click {}\n", ticksLine(pr.allTicks, cfg.shipPct, false));
        t += fmt::format("Liberacion en: {} | Debug: {}\n",
            cfg.releasePoint == 0 ? "processCommands" : "playerUpdate", cfg.debug ? "ON" : "OFF");
        t += fmt::format("Inputs retenidos: {}\n\n", waiting);

        t += "Configurar: Geode > Predictive Input Buffer > ajustes.\n";
        t += fmt::format("La tecla {} cambia Suave > Medio > Alto > Fuerte > Off.\n", cfg.cycleKey);
        if (cfg.holdKey != "None") t += fmt::format("Mantener {} pausa el buffer.\n", cfg.holdKey);
        t += "Tip: desactiva el mod original mientras probas este.\n\n";

        t += fmt::format("Hooks: processCommands {} (medios {}) | playerUpdate {}\n",
            st.c.callsPC, st.c.callsPCHalf, st.c.callsPU);
        t += fmt::format("Contadores: total {} | retenidos {} | entregados {} | directos {} | seguro {}\n",
            st.c.total, st.c.held, st.c.delivered, st.c.direct, st.c.failsafe);
        if (st.c.failsafe > 0) {
            t += "AVISO: el seguro esta soltando clicks tarde. Proba cambiar 'Liberacion en' en los ajustes.\n";
        }

        t += "\nUltimos eventos:\n";
        if (st.events.empty()) t += "  (ninguno todavia)\n";
        for (auto const& e : st.events) t += "  " + e + "\n";
        return t;
    }

#ifdef GEODE_IS_WINDOWS
    static HANDLE g_out = INVALID_HANDLE_VALUE;
    static bool g_owns = false;  // true if this mod created the console window

    static WORD paletteFor() {
        auto const& st = state();
        if (!kPresets[st.preset].enabled) return 0x4F;  // white on dark red
        if (st.holdDisabled) return 0x6F;               // white on dark yellow
        return 0x2F;                                    // white on dark green
    }
#endif

    void initConsole() {
#ifdef GEODE_IS_WINDOWS
        if (!state().cfg.consoleEnabled) return;

        g_owns = AllocConsole() != 0;
        g_out = CreateFileA(
            "CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_EXISTING, 0, nullptr
        );
        if (g_out == INVALID_HANDLE_VALUE) return;

        if (g_owns) {
            SetConsoleTitleA("Predictive Input Buffer");

            // Fit the buffer to the window so nothing scrolls.
            CONSOLE_SCREEN_BUFFER_INFO info;
            if (GetConsoleScreenBufferInfo(g_out, &info)) {
                COORD size;
                size.X = static_cast<SHORT>(info.srWindow.Right - info.srWindow.Left + 1);
                size.Y = static_cast<SHORT>(info.srWindow.Bottom - info.srWindow.Top + 1);
                SetConsoleScreenBufferSize(g_out, size);
            }

            // Text selection (QuickEdit) freezes anything that writes to the console,
            // which would freeze the game. Turn it off.
            HANDLE in = CreateFileA(
                "CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                nullptr, OPEN_EXISTING, 0, nullptr
            );
            if (in != INVALID_HANDLE_VALUE) {
                DWORD mode = 0;
                if (GetConsoleMode(in, &mode)) {
                    mode &= ~static_cast<DWORD>(ENABLE_QUICK_EDIT_MODE);
                    mode |= ENABLE_EXTENDED_FLAGS;
                    SetConsoleMode(in, mode);
                }
                CloseHandle(in);
            }

            // Closing the console window would close the game, so gray out the X.
            if (HWND hwnd = GetConsoleWindow()) {
                if (HMENU menu = GetSystemMenu(hwnd, FALSE)) {
                    DeleteMenu(menu, SC_CLOSE, MF_BYCOMMAND);
                    DrawMenuBar(hwnd);
                }
            }
        }
#endif
    }

    void renderConsole(bool force) {
#ifdef GEODE_IS_WINDOWS
        auto& st = state();
        if (g_out == INVALID_HANDLE_VALUE) return;

        double now = wallMs();
        if (!force && (!st.dirty || now - st.lastRenderMs < 250.0)) return;
        st.dirty = false;
        st.lastRenderMs = now;

        std::string text = buildPanel();

        // The console wants \r\n line endings.
        std::string crlf;
        crlf.reserve(text.size() + 64);
        for (char ch : text) {
            if (ch == '\n') crlf += '\r';
            crlf += ch;
        }

        DWORD written = 0;
        if (g_owns) {
            SetConsoleTitleA(buildTitle().c_str());

            CONSOLE_SCREEN_BUFFER_INFO info;
            if (!GetConsoleScreenBufferInfo(g_out, &info)) return;
            DWORD cells = static_cast<DWORD>(info.dwSize.X) * static_cast<DWORD>(info.dwSize.Y);
            COORD origin = {0, 0};
            WORD attr = paletteFor();

            FillConsoleOutputAttribute(g_out, attr, cells, origin, &written);
            FillConsoleOutputCharacterA(g_out, ' ', cells, origin, &written);
            SetConsoleTextAttribute(g_out, attr);
            SetConsoleCursorPosition(g_out, origin);
            WriteConsoleA(g_out, crlf.data(), static_cast<DWORD>(crlf.size()), &written, nullptr);
        } else if (force) {
            // Shared console (Geode's or another mod's): do not clear it, just append.
            WriteConsoleA(g_out, crlf.data(), static_cast<DWORD>(crlf.size()), &written, nullptr);
        }
#endif
    }
}
