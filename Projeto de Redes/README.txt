Projeto de Redes de Computadores
Programa: Jogo da Forca
Descrição: Jogo multiplayer em Python que roda no terminal, utilizando sockets TCP e threads.

Requisitos:
-> Python 3 (Obrigatório)
-> Radmin VPN (Opcional)
-> Windows ou Linux (Para execução em computadores diferentes)

Instruções de execução para um ou mais clientes no mesmo computador:
1. Tendo baixado os arquivos 'server.py', 'client.py' e 'palavras.txt', coloque-os na mesma pasta.
2. Execute 'server.py'. Se necessário, use o terminal para acessar a pasta dos conteúdos, e então use o comando "python server.py" no mesmo terminal. 
3. Execute 'client.py', uma vez para cada cliente desejado. Se necessário, abra novos terminais para cada cliente, acesse a pasta dos conteúdos e use o comando "python client.py".
4. Pronto, o jogo já está funcionando. Cada cliente deve colocar seu nome de usuário diretamente no terminal antes de participar no jogo.
-> Recomenda-se que todos os clientes coloquem o nome antes que qualquer um comece a jogar.

Instruções de execução para dois ou mais clientes em computadores diferentes:
1. Tendo baixado os arquivos 'server.py', 'client.py' e 'palavras.txt', coloque-os na mesma pasta.
2. Certifique-se que todos os computadores estão na mesma rede. Se isso não for possível, utilizar programas externos para tal.
-> Para testes, foi utilizado o Radmin VPN (https://radmin-vpn.com). Nele, crie uma rede e conecte todos os computadores na mesma rede privada.
3. Abra 'server.py' e 'client.py' com algum programa para editar texto (Bloco de Notas, Visual Studio Code, etc.).
4. Em ambos, modifique 'localhost' no início do código para o IPv4 do computador que irá ser o host do servidor. (Exemplo: HOST = '26.3.231.212')
-> Para checar o IP, use "ipconfig" ou "ifconfig" num terminal. Caso esteja usando o Radmin, o IP aparece na interface do programa, logo abaixo do nome do dispositivo.
5. Após salvar as mudanças, siga os passos 2, 3 e 4 das instruções de execução para um ou mais clientes no mesmo computador.

Como funciona e como jogar:
Com os nomes armazenados, o sistema distribui os turnos e pede uma letra por cliente a cada turno. O servidor checa se a letra está contida na palavra, e se estiver, quantas vezes aparece. O jogador que acertou a letra, recebe pontos por acerto, mas não recebe nada se errou. O turno passa para outro jogador após um enviar uma letra que ainda não tenha sido usada. A rodada continua até que a palavra seja descoberta ou até que um certo número de erros seja alcançado. Os erros são compartilhados entre todos os jogadores. O jogador pode 'chutar' a palavra para tentar adquirir pontos bônus e a completar imediatamente, apenas escrevendo ela por completo. Quando a rodada acaba, um ou mais jogadores com a maior pontuação da rodada recebem uma vitória, e outra rodada se inicia após 5 segundos. O jogo continua indeterminadamente enquanto o servidor estiver aberto.
