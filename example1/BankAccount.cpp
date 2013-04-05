#include "BankAccount.hpp"

float BankAccount::s_totalMoneyInBank = 0;

BankAccount::BankAccount(const char* owner, float balance) 
    : m_owner(owner)
    , m_balance(balance)
{
    s_totalMoneyInBank += balance;
}

const char* BankAccount::getOwnerName() const
{
    return m_owner;
}

void BankAccount::deposit(float amount)
{
    s_totalMoneyInBank += amount;
    m_balance += amount;
}

void BankAccount::withdraw(float amount)
{
    s_totalMoneyInBank -= amount;
    m_balance -= amount;
}

float BankAccount::checkBalance() const
{
    return m_balance;
}

float BankAccount::checkTotalMoneyInBank()
{
    return s_totalMoneyInBank;
}

