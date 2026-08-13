#ifndef TURBO_MICROG_MANAGER_HPP
#define TURBO_MICROG_MANAGER_HPP

#include "turbo_common.hpp"

namespace Turbo {

    struct AccountState {
        bool isMicroGActive = true;
        bool isAccountLoggedIn = false;
        std::string loggedInAccountEmail = "";
        size_t microgRamUsageMB = 38; // < 50 MB target RAM vs 1.2 GB stock GMS
    };

    class MicroGManager {
    public:
        MicroGManager();
        ~MicroGManager();

        bool InitializeMicroGCore();
        AccountState GetAccountState() const { return m_accountState; }

    private:
        AccountState m_accountState;
    };

} // namespace Turbo

#endif // TURBO_MICROG_MANAGER_HPP
