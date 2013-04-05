
alicesaccount = BankAccount.new("Alice", 100)

alicesaccount:deposit(20);
alicesaccount:deposit(30);
alicesaccount:deposit(40);

bobsaccount = BankAccount.new("Bob", 200)

bobsaccount:withdraw(10);
bobsaccount:withdraw(15);
bobsaccount:withdraw(20);

function printaccountbalance(account)
    local name = account:getOwnerName()
    local balance = account:checkBalance()
    print(string.format("%s has $%d", name, balance))
end

printaccountbalance(alicesaccount)
printaccountbalance(bobsaccount)

print(string.format("The bank has $%d", BankAccount.checkTotalMoneyInBank()))
