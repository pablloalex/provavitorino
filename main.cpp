#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

#define LIMITE_CONTAS  10
#define VERSAO_SISTEMA "1.0"

using namespace std;

struct Endereco {
    string rua;
    string cidade;
    string estado;
};

void exibirMenu();
void exibirSeparador();
double calcularImposto(double valor, double taxa);
string formatarDinheiro(double valor);
bool senhaValida(const string& senha);

class Titular {
private:
    string   nome;
    string   cpf;
    int      idade;
    Endereco endereco;

public:
    Titular() {
        nome  = "Sem nome";
        cpf   = "000.000.000-00";
        idade = 0;
    }

    Titular(string n, string c, int i, Endereco end) {
        nome     = n;
        cpf      = c;
        idade    = i;
        endereco = end;
    }

    string getNome()  const { return nome; }
    string getCpf()   const { return cpf; }
    int    getIdade() const { return idade; }

    void exibirInfo() const {
        cout << "  Nome    : " << nome << "\n";
        cout << "  CPF     : " << cpf  << "\n";
        cout << "  Idade   : " << idade << " anos\n";
        cout << "  Cidade  : " << endereco.cidade << "/" << endereco.estado << "\n";
    }
};

class Conta {
private:
    int     numero;
    double  saldo;
    double  limite;
    bool    ativa;
    string  tipo;
    string  senha;
    Titular titular;
    string  historico[50];
    int     totalTransacoes;

public:
    Conta(int num, string t, Titular tit, string sen, double limCheque) {
        numero          = num;
        tipo            = t;
        titular         = tit;
        senha           = sen;
        saldo           = 0.0;
        limite          = limCheque;
        ativa           = true;
        totalTransacoes = 0;
    }

    int     getNumero() const { return numero; }
    double  getSaldo()  const { return saldo;  }
    bool    isAtiva()   const { return ativa;  }
    string  getTipo()   const { return tipo;   }
    string  getSenha()  const { return senha;  }
    Titular getTitular()const { return titular; }

    void registrarTransacao(string descricao) {
        if (totalTransacoes < 50) {
            historico[totalTransacoes] = descricao;
            totalTransacoes++;
        } else {
            cout << "  Historico cheio.\n";
        }
    }

    bool depositar(double valor) {
        if (!ativa)    { cout << "  Conta inativa.\n";  return false; }
        if (valor <= 0){ cout << "  Valor invalido.\n"; return false; }
        saldo += valor;
        registrarTransacao("Deposito: R$ " + to_string(valor));
        return true;
    }

    bool sacar(double valor) {
        if (!ativa)    { cout << "  Conta inativa.\n";  return false; }
        if (valor <= 0){ cout << "  Valor invalido.\n"; return false; }

        double saldoDisponivel = saldo + limite;
        if (valor > saldoDisponivel) {
            cout << "  Saldo insuficiente.\n";
            return false;
        }

        saldo -= valor;

        if (saldo < 0)
            cout << "  Usando limite do cheque especial!\n";

        registrarTransacao("Saque: R$ " + to_string(valor));
        return true;
    }

    bool transferir(Conta& destino, double valor) {
        if (sacar(valor)) {
            destino.depositar(valor);
            registrarTransacao("Transferencia enviada: R$ " + to_string(valor));
            destino.registrarTransacao("Transferencia recebida: R$ " + to_string(valor));
            return true;
        }
        return false;
    }

    void exibirExtrato() const {
        cout << "\n  ==============================\n";
        cout << "  EXTRATO - Conta " << numero << " (" << tipo << ")\n";
        cout << "  Titular: " << titular.getNome() << "\n";
        cout << "  Saldo  : " << formatarDinheiro(saldo) << "\n";
        cout << "  Limite : " << formatarDinheiro(limite) << "\n";
        cout << "  Status : " << (ativa ? "Ativa" : "Inativa") << "\n";
        cout << "  ------------------------------\n";
        cout << "  HISTORICO:\n";

        if (totalTransacoes == 0) {
            cout << "  Nenhuma transacao registrada.\n";
        } else {
            for (int i = 0; i < totalTransacoes; i++) {
                cout << "  [" << (i + 1) << "] " << historico[i] << "\n";
            }
        }
        cout << "  ==============================\n";
    }

    void exibirResumo() const {
        cout << "  [" << numero << "] "
             << left << setw(20) << titular.getNome()
             << " | " << setw(10) << tipo
             << " | " << formatarDinheiro(saldo)
             << " | " << (ativa ? "Ativa" : "Inativa") << "\n";
    }

    void encerrar() { ativa = false; }
};

class Banco {
private:
    string        nome;
    vector<Conta> contas;
    Conta*        ultimaContaAcessada;
    int           proximoNumero;

public:
    Banco(string n) {
        nome                = n;
        proximoNumero       = 1001;
        ultimaContaAcessada = nullptr;
    }

    void abrirConta(string tipo, Titular tit, string senha, double limite) {
        if ((int)contas.size() >= LIMITE_CONTAS) {
            cout << "  Limite de " << LIMITE_CONTAS << " contas atingido.\n";
            return;
        }
        Conta nova(proximoNumero++, tipo, tit, senha, limite);
        contas.push_back(nova);
        cout << "  Conta aberta! Numero: " << (proximoNumero - 1) << "\n";
    }

    Conta* buscarConta(int numero) {
        for (int i = 0; i < (int)contas.size(); i++) {
            if (contas[i].getNumero() == numero) {
                ultimaContaAcessada = &contas[i];
                return &contas[i];
            }
        }
        return nullptr;
    }

    Conta* autenticar(int numero, string senha) {
        Conta* c = buscarConta(numero);
        if (c == nullptr)        { cout << "  Conta nao encontrada.\n"; return nullptr; }
        if (c->getSenha()!=senha){ cout << "  Senha incorreta.\n";      return nullptr; }
        if (!c->isAtiva())       { cout << "  Conta inativa.\n";        return nullptr; }
        return c;
    }

    void listarContas() {
        if (contas.empty()) { cout << "  Nenhuma conta cadastrada.\n"; return; }

        exibirSeparador();
        cout << "  CONTAS DO " << nome << "\n";
        exibirSeparador();

        for (const Conta& c : contas)
            c.exibirResumo();

        exibirSeparador();
        cout << "  Total: " << contas.size() << " conta(s)\n";
    }

    void relatorio() {
        double totalSaldo = 0.0;
        int    ativas     = 0;
        int    inativas   = 0;

        int i = 0;
        while (i < (int)contas.size()) {
            if (contas[i].isAtiva()) {
                totalSaldo += contas[i].getSaldo();
                ativas++;
            } else {
                inativas++;
            }
            i++;
        }

        double imposto = calcularImposto(totalSaldo, 0.015);

        cout << "\n  ==============================\n";
        cout << "  RELATORIO - " << nome << "\n";
        cout << "  ------------------------------\n";
        cout << "  Contas ativas   : " << ativas   << "\n";
        cout << "  Contas inativas : " << inativas << "\n";
        cout << "  Saldo total     : " << formatarDinheiro(totalSaldo) << "\n";
        cout << "  Imposto (1.5%)  : " << formatarDinheiro(imposto)    << "\n";
        cout << "  ==============================\n";

        if (ultimaContaAcessada != nullptr) {
            cout << "  Ultima conta acessada: "
                 << ultimaContaAcessada->getNumero() << " - "
                 << ultimaContaAcessada->getTitular().getNome() << "\n";
        }
    }

    string getNome() const { return nome; }
};

void exibirSeparador() {
    cout << "  ==============================\n";
}

double calcularImposto(double valor, double taxa) {
    return valor * taxa;
}

string formatarDinheiro(double valor) {
    char buffer[30];
    sprintf(buffer, "R$ %10.2f", valor);
    return string(buffer);
}

bool senhaValida(const string& senha) {
    if (senha.length() < 4) return false;
    for (int i = 0; i < (int)senha.length(); i++) {
        if (!isdigit(senha[i])) return false;
    }
    return true;
}

void exibirMenu() {
    cout << "\n╔════════════════════════════════╗\n";
    cout << "║        BANCO PIAUÍ " << VERSAO_SISTEMA << "         ║\n";
    cout << "╠════════════════════════════════╣\n";
    cout << "║   1. Abrir Conta               ║\n";
    cout << "║   2. Depositar                 ║\n";
    cout << "║   3. Sacar                     ║\n";
    cout << "║   4. Transferir                ║\n";
    cout << "║   5. Ver Extrato               ║\n";
    cout << "║   6. Listar Contas             ║\n";
    cout << "║   7. Encerrar Conta            ║\n";
    cout << "║   8. Relatorio Financeiro      ║\n";
    cout << "║   0. Sair                      ║\n";
    cout << "╠════════════════════════════════╣\n";
    cout << "║   Escolha:                     ║\n";
    cout << "╚════════════════════════════════╝\n";
    cout << "  Opção: ";
}

int main() {
    int    opcao    = 0;
    bool   rodando  = true;
    string nomeBanco = "Banco C++ S/A";

    Banco banco(nomeBanco);

    Endereco end1 = {"Rua das Flores, 10", "Fortaleza", "CE"};
    Endereco end2 = {"Av. Brasil, 200",    "Sao Paulo",  "SP"};
    Endereco end3 = {"Rua do Sol, 55",     "Recife",     "PE"};

    Titular t1("Maria Silva",  "111.222.333-44", 30, end1);
    Titular t2("Carlos Souza", "555.666.777-88", 45, end2);
    Titular t3("Beatriz Lima", "999.000.111-22", 28, end3);

    banco.abrirConta("Corrente", t1, "1234", 500.0);
    banco.abrirConta("Poupanca", t2, "5678", 0.0);
    banco.abrirConta("Corrente", t3, "4321", 300.0);

    do {
        exibirMenu();
        cin >> opcao;
        cin.ignore();

        switch (opcao) {

            case 1: {
                string nome, cpf, cidade, estado, rua, tipo, senha;
                int    idade;
                double limite;

                cout << "\n  -- NOVA CONTA --\n";
                cout << "  Nome     : "; getline(cin, nome);
                cout << "  CPF      : "; getline(cin, cpf);
                cout << "  Idade    : "; cin >> idade; cin.ignore();
                cout << "  Rua      : "; getline(cin, rua);
                cout << "  Cidade   : "; getline(cin, cidade);
                cout << "  Estado   : "; getline(cin, estado);
                cout << "  Tipo (Corrente/Poupanca): "; getline(cin, tipo);
                cout << "  Limite cheque especial  : "; cin >> limite; cin.ignore();

                do {
                    cout << "  Senha (4 digitos numericos): ";
                    getline(cin, senha);
                    if (!senhaValida(senha))
                        cout << "  Senha invalida! Use 4 digitos.\n";
                } while (!senhaValida(senha));

                Endereco end = {rua, cidade, estado};
                Titular  tit(nome, cpf, idade, end);
                banco.abrirConta(tipo, tit, senha, limite);
                break;
            }

            case 2: {
                int    num;
                double valor;
                string senha;

                cout << "\n  -- DEPOSITO --\n";
                cout << "  Numero da conta : "; cin >> num; cin.ignore();
                cout << "  Senha           : "; getline(cin, senha);

                Conta* c = banco.autenticar(num, senha);
                if (c != nullptr) {
                    cout << "  Valor           : R$ "; cin >> valor;
                    if (c->depositar(valor))
                        cout << "  Deposito de " << formatarDinheiro(valor) << " realizado!\n";
                }
                break;
            }

            case 3: {
                int    num;
                double valor;
                string senha;

                cout << "\n  -- SAQUE --\n";
                cout << "  Numero da conta : "; cin >> num; cin.ignore();
                cout << "  Senha           : "; getline(cin, senha);

                Conta* c = banco.autenticar(num, senha);
                if (c != nullptr) {
                    cout << "  Valor           : R$ "; cin >> valor;
                    if (c->sacar(valor))
                        cout << "  Saque de " << formatarDinheiro(valor) << " realizado!\n";
                }
                break;
            }

            case 4: {
                int    numOrig, numDest;
                double valor;
                string senha;

                cout << "\n  -- TRANSFERENCIA --\n";
                cout << "  Conta de origem  : "; cin >> numOrig; cin.ignore();
                cout << "  Senha            : "; getline(cin, senha);

                Conta* origem = banco.autenticar(numOrig, senha);
                if (origem != nullptr) {
                    cout << "  Conta de destino : "; cin >> numDest;
                    Conta* destino = banco.buscarConta(numDest);

                    if (destino == nullptr) {
                        cout << "  Conta destino nao encontrada.\n";
                    } else {
                        cout << "  Valor            : R$ "; cin >> valor;
                        if (origem->transferir(*destino, valor))
                            cout << "  Transferencia de " << formatarDinheiro(valor) << " realizada!\n";
                    }
                }
                break;
            }

            case 5: {
                int    num;
                string senha;

                cout << "\n  -- EXTRATO --\n";
                cout << "  Numero da conta : "; cin >> num; cin.ignore();
                cout << "  Senha           : "; getline(cin, senha);

                Conta* c = banco.autenticar(num, senha);
                if (c != nullptr) c->exibirExtrato();
                break;
            }

            case 6: {
                banco.listarContas();
                break;
            }

            case 7: {
                int    num;
                string senha;

                cout << "\n  -- ENCERRAR CONTA --\n";
                cout << "  Numero da conta : "; cin >> num; cin.ignore();
                cout << "  Senha           : "; getline(cin, senha);

                Conta* c = banco.autenticar(num, senha);
                if (c != nullptr) {
                    char confirma;
                    cout << "  Confirma encerramento? (s/n): "; cin >> confirma;
                    if (confirma == 's' || confirma == 'S') {
                        c->encerrar();
                        cout << "  Conta encerrada.\n";
                    } else {
                        cout << "  Operacao cancelada.\n";
                    }
                }
                break;
            }

            case 8: {
                banco.relatorio();
                break;
            }

            case 0: {
                rodando = false;
                break;
            }

            default:
                cout << "  Opcao invalida!\n";
        }

    } while (rodando);

    cout << "\n  Encerrando o sistema. Ate logo!\n";
    return 0;
}