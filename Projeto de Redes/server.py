import socket
import threading
import random
import time

HOST = '26.3.231.212'
PORT = 5555

clientes = []
enderecos = []
nomes = []
pontuacoes = {}
vitorias = {}
palavras = []
palavra = ""
palavravisivel = []
erros = 0
maxerros = 6
letrasusadas = []
turno = 0
lock = threading.Lock()

def removercliente(cliente, endereco, nome):
    global turno
    indice = clientes.index(cliente)
    if cliente in clientes:
        clientes.remove(cliente)
    if endereco in enderecos:
        enderecos.remove(endereco)
    if nome in nomes:
        nomes.remove(nome)

    if len(clientes) == 0:
        turno = 0
    elif indice < turno:
        turno -= 1
    elif indice == turno:
        turno = turno % len(clientes)

def enviaraojogador(indice, msg): # envia por índice
    try:
        clientes[indice].sendall((msg + "\n").encode())
    except:
        removercliente(clientes[indice], enderecos[indice], nomes[indice])

def broadcast(msg): # envia para todos
    for cliente in clientes:
        try:
            cliente.sendall((msg + "\n").encode())
        except:
            idx = clientes.index(cliente)
            removercliente(cliente, enderecos[idx], nomes[idx])

def pegarpalavras():
    global palavras
    with open("palavras.txt", "r", encoding="utf-8") as f:
        palavras = [linha.strip() for linha in f if linha.strip()]

def escolherpalavra():
    global palavra, palavravisivel, erros, letrasusadas
    palavra = random.choice(palavras).lower()
    palavravisivel = ["_" if c.isalpha() else c for c in palavra]
    erros = 0
    letrasusadas = []

def textos():
    estado = f"Palavra: {' '.join(palavravisivel)}\n"
    estado += f"Erros: {erros}/{maxerros}\n"
    estado += f"Letras usadas: {', '.join(letrasusadas)}\n"
    estado += f"Para desconectar do jogo, digite: sair\n"
    estado += f"Placar:\n"
    for nome in nomes:
        estado += f"-> {nome}: {vitorias.get(nome, 0)} vitórias. Tem {pontuacoes.get(nome, 0)} ponto(s) nessa rodada\n"
    return estado

def tratarmsgs(cliente, endereco):
    global turno, erros, palavra, palavravisivel, pontuacoes

    with lock:
        try:
            cliente.sendall("Digite seu nome: ".encode())
            nome = cliente.recv(2048).decode().strip()
            print(f"[NOME RECEBIDO] {nome} de {endereco}")
            nomes.append(nome)
            pontuacoes[nome] = 0
            vitorias[nome] = 0
        except:
            removercliente(cliente, endereco, nome)
            cliente.close()
            return

    cliente.sendall("Conectado ao servidor.\n".encode())

    while True:
        try:
            indice = clientes.index(cliente)

            if indice != turno:
                enviaraojogador(indice, f"{textos()}\nAguarde, é a vez de {nomes[turno]}.")
                time.sleep(1)
                continue
            else:
                enviaraojogador(indice, f"{textos()}\nSua vez! Envie uma letra ou chute a palavra (+5 pontos):")

            dados = cliente.recv(2048).decode().strip()
            if not dados:
                break

            if len(dados) == 1 and dados.isalpha():
                letra = dados.lower()

                with lock:
                    if letra in letrasusadas:
                        enviaraojogador(indice, f"{textos()}\nLetra {letra} já foi usada.")
                        continue

                    letrasusadas.append(letra)

                    if letra in palavra:
                        acertos = 0
                        for i, c in enumerate(palavra):
                            if c == letra and palavravisivel[i] == "_":
                                palavravisivel[i] = letra
                                acertos += 1
                        if acertos > 0:
                            pontuacoes[nome] = pontuacoes.get(nome, 0) + acertos
                        broadcast(f"{textos()}\n{nomes[indice]} acertou a letra '{letra}'!")
                    else:
                        erros += 1
                        broadcast(f"{textos()}\n{nomes[indice]} errou a letra '{letra}'!")

                if "_" not in palavravisivel:
                    jogadoracerto = nomes[indice]
                    maiorpontuacao = max(pontuacoes.values())
                    vencedores = [nome for nome, pontos in pontuacoes.items() if pontos == maiorpontuacao]
                    for nomevencedor in vencedores:
                        vitorias[nomevencedor] += 1
                    if len(vencedores) == 1:
                        broadcast(f"VITÓRIA! {jogadoracerto} acertou a palavra '{palavra}'.\n{vencedores[0]} ganhou essa rodada com {maiorpontuacao} pontos!\nA próxima rodada começa em 5 segundos.")
                    else:
                        broadcast(f"EMPATE! {jogadoracerto} acertou a palavra '{palavra}'.\n{', '.join(vencedores)} ganharam essa rodada com {maiorpontuacao} pontos!\nA próxima rodada começa em 5 segundos.")
                    pontuacoes = {}
                    turno = (turno + 1) % len(clientes)
                    escolherpalavra()
                    continue

                if erros >= maxerros:
                    broadcast(f"DERROTA! A palavra era: '{palavra}'. A próxima rodada começa em 5 segundos.")
                    pontuacoes = {}
                    turno = (turno + 1) % len(clientes)
                    escolherpalavra()
                    continue

                turno = (turno + 1) % len(clientes)

            else:
                with lock:
                    if dados.lower() == palavra.lower():
                        jogadoracerto = nomes[indice]
                        palavravisivel[:] = list(palavra)
                        pontuacoes[jogadoracerto] = pontuacoes.get(jogadoracerto, 0) + 5 # +5 pontos para quem acertou o chute
                        maiorpontuacao = max(pontuacoes.values())
                        vencedores = [nome for nome, pontos in pontuacoes.items() if pontos == maiorpontuacao]
                        for nomevencedor in vencedores:
                            vitorias[nomevencedor] += 1
                        if len(vencedores) == 1:
                            broadcast(f"VITÓRIA! {jogadoracerto} acertou a palavra '{palavra}'.\n{vencedores[0]} ganhou essa rodada com {maiorpontuacao} pontos!\nA próxima rodada começa em 5 segundos.")
                        else:
                            broadcast(f"EMPATE! {jogadoracerto} acertou a palavra '{palavra}'.\n{', '.join(vencedores)} ganharam essa rodada com {maiorpontuacao} pontos!\nA próxima rodada começa em 5 segundos.")
                        pontuacoes = {}
                        turno = (turno + 1) % len(clientes)
                        escolherpalavra()
                        continue
                    else:
                        erros += 1
                        if erros >= maxerros:
                            broadcast(f"DERROTA! A palavra era: '{palavra}'. A próxima rodada começa em 5 segundos.")
                            pontuacoes = {}
                            turno = (turno + 1) % len(clientes)
                            escolherpalavra()
                            continue
                        else:
                            broadcast(f"{textos()}\n{nomes[indice]} tentou chutar a palavra e errou.")
                            turno = (turno + 1) % len(clientes)

        except Exception as e:
            print(f"[ERRO] com o jogador {nome}: {e}")
            break

    removercliente(cliente, endereco, nome)
    cliente.close()
    broadcast(f"{nome} saiu do jogo.")

def main():
    pegarpalavras()
    escolherpalavra()
    servidor = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        servidor.bind((HOST, PORT))
        servidor.listen()
        print(f"[SERVIDOR] Ouvindo em {HOST}:{PORT}...")
    except:
        return print("\nNão foi possível iniciar o servidor.\n")

    try:
        while True:
            cliente, endereco = servidor.accept()
            print(f"[NOVA CONEXÃO] {endereco}")
            clientes.append(cliente)
            enderecos.append(endereco)

            thread = threading.Thread(target=tratarmsgs, args=(cliente, endereco))
            thread.start()
    except KeyboardInterrupt:
        print("\n[SERVIDOR] Encerrado manualmente.")
        servidor.close()

main()