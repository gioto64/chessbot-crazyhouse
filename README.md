# Crazyhouse Chessbot

This is a chessbot programmed to play the Crazyhouse variant of chess. The bot utilizes various advanced techniques, including Iterative Deepening, Transposition Tables, Negascout (Principal Variation Search), and Precalculated Piece Values based on position to enhance its gameplay and decision-making. The bot is compatible with xboard. 

## Features

- **Iterative Deepening**: The chessbot employs the Iterative Deepening search technique to explore deeper levels of the game tree gradually. This allows it to prioritize promising moves and improve its understanding of the game's complexity.

- **Transposition Tables**: Transposition tables are used to store previously evaluated positions along with their associated scores. This helps the bot avoid redundant calculations and improves its search efficiency.

- **Negascout (Principal Variation Search)**: The Negascout algorithm, also known as Principal Variation Search, is employed to further optimize the search process. It reduces the number of nodes searched by making use of the upper and lower bounds obtained from previous searches.

- **Precalculated Piece Values**: The bot uses precalculated tables to evaluate the relative values of pieces based on their positions. This allows the bot to assess the strength of each piece more accurately and make informed decisions during gameplay.

- **Elo Rating**: The estimate ELO of this bot is around 1600 ELO.

## How to Use

1. Clone the repository to your local machine:

    ```bash
    git clone https://github.com/yourusername/chessbot-crazyhouse.git
    ```

2. Navigate to the project directory:
    ```bash
    cd chessbot-crazyhouse 
    ```

3. Build the Crazyhouse Chessbot by running:
    ```bash
    make build
    ```

4. In order to run the Crazyhouse Chessbot, please use the command: 
    ```bash
    make run
    ```
To play with a graphical interface, simply use xboard.

## License

This project is licensed under the [MIT License](LICENSE).
