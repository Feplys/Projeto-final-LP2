Chat Multiusuário Concorrente (Etapas 1 e 2)

Estrutura do Projeto
O projeto está organizado da seguinte forma para manter o código limpo e modular:
/
├── Makefile                # Automatiza a compilação, testes e execução
├── README.md               # Esta documentação
├── include/                # Arquivos de cabeçalho (.h)
│   ├── libtslog.h
│   ├── chat_common.h
│   ├── simple_chat_server.h
│   └── ...
├── src/                    # Arquivos de implementação (.cpp)
│   ├── libtslog.cpp
│   ├── simple_chat_server.cpp
│   ├── chat_server_main.cpp
│   └── ...
├── scripts/                # Scripts de teste e demonstração
│   ├── quick_test.sh
│   ├── test_multiple_clients.sh
│   └── visual_demo.sh
├── bin/                    # Executáveis (gerado pela compilação)
│   ├── chat_server
│   └── chat_client
└── build/                  # Arquivos objeto (gerado pela compilação)


Como Compilar e Executar:
O Makefile automatiza todo o processo. Abra um terminal na raiz do projeto e use os seguintes comandos:

make all =(Recomendado) Compila o projeto inteiro (servidor e cliente).
make clean=Limpa todos os arquivos gerados pela compilação (bin/, build/, *.log).
make etapa1=Compila e testa apenas a biblioteca libtslog da Etapa 1.
make etapa2=Compila tudo e executa o teste rápido da Etapa 2.

Execução Manual (para Testar Conversando)
Para testar o chat manualmente, você precisará de pelo menos dois terminais.
No Terminal 1, inicie o servidor:
make demo-server

O servidor ficará rodando e esperando por conexões.
No Terminal 2 (e 3, 4...), inicie os clientes:
make demo-client

O programa vai pedir seu nome de usuário. Depois, é só digitar as mensagens e apertar Enter.

Scripts de Teste:

Os scripts são a forma de provar que o sistema funciona automaticamente, como pedido na Etapa 2. Eles estão na pasta scripts/ e podem ser executados facilmente pelo Makefile.
1. Teste Rápido (quick_test.sh)
Este é o principal teste de validação da Etapa 2.
O que faz? Inicia o servidor em segundo plano, conecta 3 clientes "bots", faz cada um enviar 3 mensagens e, no final, lê o arquivo de log do servidor para verificar se o número de conexões e mensagens retransmitidas está correto.
Como rodar?
make test-etapa2
Resultado esperado: Uma mensagem 🎉 TESTE PASSOU! no final.


2. Teste de Estresse (test_multiple_clients.sh)
Este é um teste mais pesado para verificar a estabilidade do servidor com mais usuários.
O que faz? Similar ao teste rápido, mas simula mais clientes (5 por padrão) enviando mais mensagens (8 por padrão). Ele mede a taxa de sucesso e o tempo total, dando um relatório de performance.
Como rodar?
make test-stress


Resultado esperado: Um relatório detalhado e uma mensagem de 🎉 TESTE PASSOU! se a taxa de sucesso for alta.
3. Demonstração Visual (visual_demo.sh)
Este script é para você ver o chat acontecendo em tempo real.
O que faz? Abre múltiplas janelas de terminal automaticamente: uma para o servidor e várias para os clientes "bots". Os bots ficam conversando sozinhos, enviando mensagens pré-definidas.
Como rodar?
make demo-visual



