#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <memory>
#include <limits>
#include <algorithm>
#include <cctype>

#define VERSAO_SISTEMA "2.0"

using namespace std;

// ==================== CONSTANTES ====================
namespace Constantes {
    const int LIMITE_CONTAS = 10;
    const int MAX_TRANSACOES = 50;
    const int TAMANHO_SENHA = 4;
    const double TAXA_IMPOSTO = 0.015;
    const int NUMERO_INICIAL = 1001;
}

// ==================== UTILITÁRIOS ====================
class Validador {
public:
    static bool valorPositivo(double valor, const string& mensagem = "Valor") {
        if (valor <= 0) {
            cout << "  ERRO: " << mensagem << " deve ser positivo.\n";
            return false;
        }
        return true;
    }

    static bool senhaValida(const string& senha) {
        if (senha.length() != Constantes::TAMANHO_SENHA) {
            cout << "  ERRO: Senha deve ter exatamente " << Constantes::TAMANHO_SENHA << " digitos.\n";
            return false;
        }
        for (char c : senha) {
            if (!isdigit(c)) {
                cout << "  ERRO: Senha deve conter apenas numeros.\n";
                return false;
            }
        }
        return true;
    }

    static bool idadeValida(int idade) {
        if (idade <= 0 || idade > 150) {
            cout << "  ERRO: Idade invalida (deve ser entre 1 e 150 anos).\n";
            return false;
        }
        return true;
    }

    static bool cpfValido(const string& cpf) {
        // Validação básica de formato (apenas para exemplo)
        if (cpf.length() < 11 || cpf.length() > 14) {
            cout << "  ERRO: CPF invalido.\n";
            return false;
        }
        return true;
    }
};

class Formatador {
public:
    static string dinheiro(double valor) {
        char buffer[30];
        sprintf(buffer, "R$ %10.2f", valor);
        return string(buffer);
    }

    static void separador() {
        cout << "  ==============================\n";
    }

    static void cabecalho(const string& titulo) {
        cout << "\n  ===== " << titulo << " =====\n";
    }
};

// ==================== CLASSES DO DOMÍNIO ====================
class Endereco {
private:
    string rua;
    string cidade;
    string estado;

public:
    Endereco() : rua("Nao informado"), cidade("Nao informado"), estado("XX") {}

    Endereco(string r, string c, string e) 
        : rua(r), cidade(c), estado(e) {}

    // Getters const
    string getRua() const { return rua; }
    string getCidade() const { return cidade; }
    string getEstado() const { return estado; }

    void exibir() const {
        cout << "    " << rua << ", " << cidade << "/" << estado << "\n";
    }
};

class Titular {
private:
    string nome;
    string cpf;
    int idade;
    Endereco endereco;

public:
    // Construtor padrão
    Titular() : nome("Sem nome"), cpf("000.000.000-00"), idade(0) {}

    // Construtor principal com validações
    Titular(string n, string c, int i, Endereco end) 
        : nome(n), cpf(c), idade(i), endereco(end) {
        // Validações em tempo de construção
        if (!Validador::cpfValido(cpf)) cpf = "000.000.000-00";
        if (!Validador::idadeValida(idade)) idade = 0;
    }

    // Getters const (protegem os dados)
    string getNome() const { return nome; }
    string getCpf() const { return cpf; }
    int getIdade() const { return idade; }
    Endereco getEndereco() const { return endereco; }

    void exibirInfo() const {
        cout << "  Titular:\n";
        cout << "    Nome    : " << nome << "\n";
        cout << "    CPF     : " << cpf << "\n";
        cout << "    Idade   : " << idade << " anos\n";
        cout << "    Endereco: ";
        endereco.exibir();
    }
};

// ==================== INTERFACE E CLASSES BASE ====================
class Transacionavel {
public:
    virtual bool depositar(double valor) = 0;
    virtual bool sacar(double valor) = 0;
    virtual double getSaldoDisponivel() const = 0;
    virtual ~Transacionavel() = default;
};

// ==================== CLASSE PRINCIPAL CONTA ====================
class Conta : public Transacionavel {
private:
    int numero;
    double saldo;
    double limiteChequeEspecial;
    bool ativa;
    string tipo;
    string senha;
    Titular titular;
    vector<string> historico;  // Usando vector em vez de array fixo

public:
    // Construtor com validações
    Conta(int num, string t, Titular tit, string sen, double limite) 
        : numero(num), saldo(0.0), ativa(true), tipo(t), titular(tit) {
        
        // Validar senha
        senha = senhaValida(sen) ? sen : "0000";
        
        // Validar limite (não pode ser negativo)
        limiteChequeEspecial = (limite >= 0) ? limite : 0.0;
        
        historico.reserve(Constantes::MAX_TRANSACOES);
    }

    // Getters const
    int getNumero() const { return numero; }
    double getSaldo() const { return saldo; }
    double getLimite() const { return limiteChequeEspecial; }
    bool isAtiva() const { return ativa; }
    string getTipo() const { return tipo; }
    Titular getTitular() const { return titular; }
    
    // Implementação da interface
    double getSaldoDisponivel() const override {
        return saldo + limiteChequeEspecial;
    }

    bool depositar(double valor) override {
        if (!validarOperacao("Deposito", valor)) return false;
        
        saldo += valor;
        registrarTransacao("Deposito: +" + Formatador::dinheiro(valor));
        return true;
    }

    bool sacar(double valor) override {
        if (!validarOperacao("Saque", valor)) return false;

        if (valor > getSaldoDisponivel()) {
            cout << "  ERRO: Saldo insuficiente.\n";
            cout << "         Disponivel: " << Formatador::dinheiro(getSaldoDisponivel()) << "\n";
            return false;
        }

        saldo -= valor;
        
        if (saldo < 0) {
            cout << "  AVISO: Usando cheque especial!\n";
            cout << "         Saldo apos saque: " << Formatador::dinheiro(saldo) << "\n";
        }

        registrarTransacao("Saque: -" + Formatador::dinheiro(valor));
        return true;
    }

    bool transferir(Conta& destino, double valor) {
        if (!validarOperacao("Transferencia", valor)) return false;
        
        if (&destino == this) {
            cout << "  ERRO: Nao pode transferir para a mesma conta.\n";
            return false;
        }

        if (!destino.isAtiva()) {
            cout << "  ERRO: Conta destino inativa.\n";
            return false;
        }

        if (sacar(valor)) {
            destino.depositar(valor);
            registrarTransacao("Transferencia enviada: -" + Formatador::dinheiro(valor) + 
                             " para conta " + to_string(destino.getNumero()));
            destino.registrarTransacao("Transferencia recebida: +" + Formatador::dinheiro(valor) + 
                                      " da conta " + to_string(numero));
            return true;
        }
        return false;
    }

    void exibirExtrato() const {
        Formatador::separador();
        cout << "  EXTRATO - Conta " << numero << " (" << tipo << ")\n";
        titular.exibirInfo();
        cout << "  ------------------------------\n";
        cout << "  Saldo atual : " << Formatador::dinheiro(saldo) << "\n";
        cout << "  Limite      : " << Formatador::dinheiro(limiteChequeEspecial) << "\n";
        cout << "  Disponivel  : " << Formatador::dinheiro(getSaldoDisponivel()) << "\n";
        cout << "  Status      : " << (ativa ? "Ativa" : "Inativa") << "\n";
        cout << "  ------------------------------\n";
        cout << "  HISTORICO:\n";

        if (historico.empty()) {
            cout << "    Nenhuma transacao registrada.\n";
        } else {
            for (size_t i = 0; i < historico.size(); i++) {
                cout << "    [" << (i + 1) << "] " << historico[i] << "\n";
            }
        }
        Formatador::separador();
    }

    void exibirResumo() const {
        cout << "  [" << numero << "] "
             << left << setw(20) << titular.getNome()
             << " | " << setw(10) << tipo
             << " | " << Formatador::dinheiro(saldo)
             << " | " << (ativa ? "Ativa   " : "Inativa ");
        
        if (saldo < 0) cout << " [!] Usando cheque especial";
        cout << "\n";
    }

    bool autenticar(const string& senhaInformada) const {
        return ativa && senha == senhaInformada;
    }

    void encerrar() { 
        if (saldo != 0) {
            cout << "  AVISO: Conta encerrada com saldo " << Formatador::dinheiro(saldo) << "\n";
        }
        ativa = false; 
    }

    bool podeSerEncerrada() const {
        return saldo == 0;
    }

private:
    bool validarOperacao(const string& operacao, double valor) const {
        if (!ativa) {
            cout << "  ERRO: Conta inativa.\n";
            return false;
        }
        if (!Validador::valorPositivo(valor, operacao)) {
            return false;
        }
        return true;
    }

    void registrarTransacao(const string& descricao) {
        if (historico.size() < Constantes::MAX_TRANSACOES) {
            historico.push_back(descricao);
        } else {
            // Remove a mais antiga e adiciona a nova
            historico.erase(historico.begin());
            historico.push_back(descricao);
            cout << "  AVISO: Historico cheio. Transacao mais antiga removida.\n";
        }
    }

    static bool senhaValida(const string& senha) {
        return Validador::senhaValida(senha);
    }
};

// ==================== CLASSE BANCO (GERENCIADOR) ====================
class Banco {
private:
    string nome;
    vector<unique_ptr<Conta>> contas;
    Conta* ultimaContaAcessada;
    int proximoNumero;

public:
    Banco(string n) : nome(n), ultimaContaAcessada(nullptr), proximoNumero(Constantes::NUMERO_INICIAL) {
        contas.reserve(Constantes::LIMITE_CONTAS);
    }

    bool abrirConta(string tipo, Titular tit, string senha, double limite) {
        if (contas.size() >= Constantes::LIMITE_CONTAS) {
            cout << "  ERRO: Limite de " << Constantes::LIMITE_CONTAS << " contas atingido.\n";
            return false;
        }

        auto novaConta = make_unique<Conta>(proximoNumero++, tipo, tit, senha, limite);
        cout << "  Conta aberta com sucesso!\n";
        cout << "  Numero: " << (proximoNumero - 1) << "\n";
        
        contas.push_back(move(novaConta));
        return true;
    }

    Conta* buscarConta(int numero) {
        for (auto& conta : contas) {
            if (conta->getNumero() == numero) {
                ultimaContaAcessada = conta.get();
                return conta.get();
            }
        }
        return nullptr;
    }

    Conta* autenticar(int numero, const string& senha) {
        Conta* conta = buscarConta(numero);
        
        if (!conta) {
            cout << "  ERRO: Conta nao encontrada.\n";
            return nullptr;
        }
        
        if (!conta->autenticar(senha)) {
            cout << "  ERRO: Senha incorreta.\n";
            return nullptr;
        }
        
        return conta;
    }

    void listarContas() const {
        if (contas.empty()) {
            cout << "  Nenhuma conta cadastrada.\n";
            return;
        }

        Formatador::separador();
        cout << "  CONTAS DO " << nome << "\n";
        Formatador::separador();

        int ativas = 0, inativas = 0;
        for (const auto& conta : contas) {
            conta->exibirResumo();
            if (conta->isAtiva()) ativas++; else inativas++;
        }

        Formatador::separador();
        cout << "  Total: " << contas.size() << " conta(s) ";
        cout << "(" << ativas << " ativas, " << inativas << " inativas)\n";
    }

    void relatorioFinanceiro() const {
        double totalSaldo = 0.0;
        int ativas = 0, inativas = 0, negativo = 0;

        for (const auto& conta : contas) {
            if (conta->isAtiva()) {
                double saldo = conta->getSaldo();
                totalSaldo += saldo;
                ativas++;
                if (saldo < 0) negativo++;
            } else {
                inativas++;
            }
        }

        double imposto = totalSaldo * Constantes::TAXA_IMPOSTO;

        Formatador::cabecalho("RELATORIO FINANCEIRO");
        cout << "  Banco: " << nome << "\n";
        Formatador::separador();
        cout << "  Contas ativas        : " << ativas << "\n";
        cout << "  Contas inativas      : " << inativas << "\n";
        cout << "  Contas com saldo neg.: " << negativo << "\n";
        cout << "  Saldo total          : " << Formatador::dinheiro(totalSaldo) << "\n";
        cout << "  Imposto (1.5%)       : " << Formatador::dinheiro(imposto) << "\n";
        Formatador::separador();

        if (ultimaContaAcessada) {
            cout << "  Ultima conta acessada: "
                 << ultimaContaAcessada->getNumero() << " - "
                 << ultimaContaAcessada->getTitular().getNome() << "\n";
        }
    }

    string getNome() const { return nome; }
    size_t getTotalContas() const { return contas.size(); }
};

// ==================== INTERFACE DO USUÁRIO ====================
class InterfaceBanco {
private:
    Banco banco;

    void limparTela() const {
        // Limpa a tela de forma portátil
        cout << string(50, '\n');
    }

    void pausar() const {
        cout << "\n  Pressione Enter para continuar...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }

    void exibirMenu() const {
        cout << "\n╔════════════════════════════════════╗\n";
        cout << "║        BANCO PIAUÍ v" << VERSAO_SISTEMA << "            ║\n";
        cout << "╠════════════════════════════════════╣\n";
        cout << "║   1. Abrir Conta                   ║\n";
        cout << "║   2. Depositar                     ║\n";
        cout << "║   3. Sacar                         ║\n";
        cout << "║   4. Transferir                    ║\n";
        cout << "║   5. Ver Extrato                   ║\n";
        cout << "║   6. Listar Contas                 ║\n";
        cout << "║   7. Encerrar Conta                ║\n";
        cout << "║   8. Relatorio Financeiro          ║\n";
        cout << "║   0. Sair                          ║\n";
        cout << "╚════════════════════════════════════╝\n";
        cout << "  Opcao: ";
    }

    // Métodos para cada operação
    void operacaoAbrirConta() {
        Formatador::cabecalho("ABRIR CONTA");

        string nome, cpf, cidade, estado, rua, tipo, senha;
        int idade;
        double limite;

        cout << "  Nome do titular: "; getline(cin, nome);
        if (nome.empty()) {
            cout << "  ERRO: Nome obrigatorio.\n";
            return;
        }

        cout << "  CPF: "; getline(cin, cpf);
        if (!Validador::cpfValido(cpf)) return;

        cout << "  Idade: "; cin >> idade; cin.ignore();
        if (!Validador::idadeValida(idade)) return;

        cout << "  Endereco:\n";
        cout << "    Rua: "; getline(cin, rua);
        cout << "    Cidade: "; getline(cin, cidade);
        cout << "    Estado: "; getline(cin, estado);

        cout << "  Tipo (Corrente/Poupanca): "; getline(cin, tipo);
        if (tipo != "Corrente" && tipo != "Poupanca") {
            cout << "  ERRO: Tipo invalido. Usando Corrente.\n";
            tipo = "Corrente";
        }

        cout << "  Limite cheque especial: R$ "; cin >> limite; cin.ignore();
        if (limite < 0) {
            cout << "  ERRO: Limite nao pode ser negativo. Definindo como 0.\n";
            limite = 0;
        }

        do {
            cout << "  Senha (" << Constantes::TAMANHO_SENHA << " digitos numericos): ";
            getline(cin, senha);
        } while (!Validador::senhaValida(senha));

        Endereco end(rua, cidade, estado);
        Titular tit(nome, cpf, idade, end);

        banco.abrirConta(tipo, tit, senha, limite);
    }

    void operacaoDepositar() {
        Formatador::cabecalho("DEPOSITO");

        int num;
        double valor;
        string senha;

        cout << "  Numero da conta: "; cin >> num; cin.ignore();
        cout << "  Senha: "; getline(cin, senha);

        Conta* conta = banco.autenticar(num, senha);
        if (!conta) return;

        cout << "  Valor do deposito: R$ "; cin >> valor;
        if (Validador::valorPositivo(valor, "Deposito")) {
            if (conta->depositar(valor)) {
                cout << "  Deposito realizado com sucesso!\n";
                cout << "  Novo saldo: " << Formatador::dinheiro(conta->getSaldo()) << "\n";
            }
        }
    }

    void operacaoSacar() {
        Formatador::cabecalho("SAQUE");

        int num;
        double valor;
        string senha;

        cout << "  Numero da conta: "; cin >> num; cin.ignore();
        cout << "  Senha: "; getline(cin, senha);

        Conta* conta = banco.autenticar(num, senha);
        if (!conta) return;

        cout << "  Saldo disponivel: " << Formatador::dinheiro(conta->getSaldoDisponivel()) << "\n";
        cout << "  Valor do saque: R$ "; cin >> valor;

        if (Validador::valorPositivo(valor, "Saque")) {
            if (conta->sacar(valor)) {
                cout << "  Saque realizado com sucesso!\n";
                cout << "  Novo saldo: " << Formatador::dinheiro(conta->getSaldo()) << "\n";
            }
        }
    }

    void operacaoTransferir() {
        Formatador::cabecalho("TRANSFERENCIA");

        int numOrigem, numDestino;
        double valor;
        string senha;

        cout << "  Conta de origem: "; cin >> numOrigem; cin.ignore();
        cout << "  Senha: "; getline(cin, senha);

        Conta* origem = banco.autenticar(numOrigem, senha);
        if (!origem) return;

        cout << "  Conta de destino: "; cin >> numDestino;
        Conta* destino = banco.buscarConta(numDestino);

        if (!destino) {
            cout << "  ERRO: Conta destino nao encontrada.\n";
            return;
        }

        cout << "  Saldo disponivel origem: " << Formatador::dinheiro(origem->getSaldoDisponivel()) << "\n";
        cout << "  Valor da transferencia: R$ "; cin >> valor;

        if (Validador::valorPositivo(valor, "Transferencia")) {
            if (origem->transferir(*destino, valor)) {
                cout << "  Transferencia realizada com sucesso!\n";
            }
        }
    }

    void operacaoExtrato() {
        Formatador::cabecalho("EXTRATO");

        int num;
        string senha;

        cout << "  Numero da conta: "; cin >> num; cin.ignore();
        cout << "  Senha: "; getline(cin, senha);

        Conta* conta = banco.autenticar(num, senha);
        if (conta) {
            conta->exibirExtrato();
        }
    }

    void operacaoListarContas() {
        Formatador::cabecalho("LISTA DE CONTAS");
        banco.listarContas();
    }

    void operacaoEncerrarConta() {
        Formatador::cabecalho("ENCERRAR CONTA");

        int num;
        string senha;
        char confirmacao;

        cout << "  Numero da conta: "; cin >> num; cin.ignore();
        cout << "  Senha: "; getline(cin, senha);

        Conta* conta = banco.autenticar(num, senha);
        if (!conta) return;

        if (!conta->podeSerEncerrada()) {
            cout << "  AVISO: A conta tem saldo de " << Formatador::dinheiro(conta->getSaldo()) << "\n";
            cout << "  Para encerrar, o saldo deve ser zero.\n";
            return;
        }

        cout << "  Confirma encerramento da conta " << num << "? (S/N): ";
        cin >> confirmacao;

        if (toupper(confirmacao) == 'S') {
            conta->encerrar();
            cout << "  Conta encerrada com sucesso.\n";
        } else {
            cout << "  Operacao cancelada.\n";
        }
    }

    void operacaoRelatorio() {
        banco.relatorioFinanceiro();
    }

public:
    InterfaceBanco(const string& nomeBanco) : banco(nomeBanco) {
        // Dados iniciais para teste
        inicializarDadosTeste();
    }

    void inicializarDadosTeste() {
        vector<Endereco> enderecos = {
            {"Rua das Flores, 10", "Fortaleza", "CE"},
            {"Av. Brasil, 200", "Sao Paulo", "SP"},
            {"Rua do Sol, 55", "Recife", "PE"}
        };

        vector<Titular> titulares = {
            Titular("Maria Silva", "111.222.333-44", 30, enderecos[0]),
            Titular("Carlos Souza", "555.666.777-88", 45, enderecos[1]),
            Titular("Beatriz Lima", "999.000.111-22", 28, enderecos[2])
        };

        banco.abrirConta("Corrente", titulares[0], "1234", 500.0);
        banco.abrirConta("Poupanca", titulares[1], "5678", 0.0);
        banco.abrirConta("Corrente", titulares[2], "4321", 300.0);

        // Faz alguns depositos iniciais
        Conta* conta = banco.buscarConta(1001);
        if (conta) conta->depositar(1000.0);

        conta = banco.buscarConta(1002);
        if (conta) conta->depositar(2000.0);

        conta = banco.buscarConta(1003);
        if (conta) conta->depositar(500.0);
    }

    void executar() {
        int opcao = 0;

        do {
            limparTela();
            exibirMenu();
            cin >> opcao;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            switch (opcao) {
                case 1: operacaoAbrirConta(); break;
                case 2: operacaoDepositar(); break;
                case 3: operacaoSacar(); break;
                case 4: operacaoTransferir(); break;
                case 5: operacaoExtrato(); break;
                case 6: operacaoListarContas(); break;
                case 7: operacaoEncerrarConta(); break;
                case 8: operacaoRelatorio(); break;
                case 0: 
                    cout << "\n  Encerrando o sistema. Ate logo!\n";
                    break;
                default:
                    cout << "  Opcao invalida!\n";
            }

            if (opcao != 0) pausar();

        } while (opcao != 0);
    }
};

// ==================== MAIN ====================
int main() {
    try {
        InterfaceBanco interface("Banco C++ S/A");
        interface.executar();
    } catch (const exception& e) {
        cerr << "Erro fatal: " << e.what() << endl;
        return 1;
    }

    return 0;
}
