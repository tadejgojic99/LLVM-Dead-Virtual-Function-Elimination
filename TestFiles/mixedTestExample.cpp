// Ovaj primer pokazuje rad vise optimizacija na jednom primeru

// Funkcija virtual_foo ce biti obrisana zato sto je prazna (Moze se stici do
// nje preko virtual_bar pa ona nije unreachable) Funkcija virtual_bar ce biti
// obrisana zato sto vraca const vrednost Funkcija virtual_koo ce biti obrisana
// zato sto nije pozvana

class Base {
public:
  virtual void virtual_foo() {}
  virtual int virtual_bar() {
    Base ptr;
    ptr.virtual_foo();
    return 5;
  }
  virtual bool virtual_koo() {
    bool x = true;
    return x & true;
  }
};

class testClass : public Base {
public:
};

int main() {
  Base ptr;
  ptr.virtual_bar();
  return 0;
}