# MMU
## Definição
  O programa simula o gerenciamento de memória de um sistema operacional, com uma MMU responsável por manter a tabela de páginas virtuais e o mapeamento entre páginas e frames.
  
  ![image](https://user-images.githubusercontent.com/63256286/204063997-8e8f7d8c-8cbe-4902-a8a1-9bfc86dfeaff.png)

### Espeficicações
  - O tamanho da memória principal é de 64K;
  - O tamanho da memória virtual é de 1MB;
  - Páginas e frames são divididos em blocos de 8K;

## Tecnologias
  - Linguagem C 
  
## Como rodar
  - Clonar o projeto
  - Compilador GCC
    - No terminal:
    
    ![image](https://user-images.githubusercontent.com/63256286/204064174-b24e9ec7-fefb-4492-bb79-03cae96a1f5c.png)
  - Executar no terminal: 
  
    ![image](https://user-images.githubusercontent.com/63256286/204064202-d5a55785-d988-40c5-bcf9-8a1814aae31f.png)

## Features
  - Gerar processo de um tamanho aletório com threads
  - Dividir processo em páginas 
  - Adicionar páginas na memória lógica, de forma sequencial
  - Adicionar os frames na memória física conforme solicitação
  - Política de substituição de páginas através de LRU, onde o horário é considerado. 
  - Verificar se exitem frames livres na memória física
  - Verificar se o frames já está na memória física, se estiver o horário do endereço correspondente na tabela de página é atualizado.
  - Transformar número decimal em binário
  - Impressão das memórias fisica e lógica, e da MMU
