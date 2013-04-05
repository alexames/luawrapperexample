#ifndef EXAMPLE_HPP_
#define EXAMPLE_HPP_

#include <string>
#include <iostream>

class BankAccount
{
public:
    BankAccount(const char* owner, float balance);

    const char* getOwnerName() const;
    void deposit(float amount);
    void withdraw(float amount);
    float checkBalance() const;
    
    static float checkTotalMoneyInBank();
    
private:
    const char* m_owner;
    float m_balance;
    static float s_totalMoneyInBank;
};

#endif // EXAMPLE_HPP_
