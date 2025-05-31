import random
sequencias = 50
caracteres = 100
bases = ['A', 'T', 'C', 'G']
with open('input.txt', 'w') as file:
    for _ in range(sequencias):
        length = random.randint(1, caracteres - 1)
        dna = ''.join(random.choices(bases, k=length))
        file.write(dna + '\n')