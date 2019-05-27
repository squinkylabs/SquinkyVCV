
#pragma once
#include <memory>
/**
 * Implements radio button from separate buttons.
 * clients hold strong pointers to us.
 * we hold weak ptrs to clients
 */

template<class TButton>
class ToggleManager2 : public std::enable_shared_from_this<ToggleManager2<TButton>>
{
public:
    void registerClient(TButton*);
    void go(TButton*);
private:
    std::vector<TButton*> clients;
};

template<class TButton>
inline void ToggleManager2<TButton>::registerClient(TButton* button)
{
    clients.push_back(button);
    std::shared_ptr<ToggleManager2<TButton>> mgr = this->shared_from_this(); 
    button->registerManager(mgr);
}

template<class TButton>
inline void ToggleManager2<TButton>::go(TButton* client)
{
    for (auto cl : clients) {
        TButton* pc = cl;
        if (pc != client) {
            pc->turnOff();
        }
    }
}