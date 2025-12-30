import matplotlib.pyplot as plt

# Path to your txt file
filename = "foraging.txt"

clock = []
walking = []
resting = []
collected_food = []

# Read file
with open(filename, "r") as f:
    for line in f:
        if line.strip() == "" or line.startswith("#"):
            continue

        parts = line.split()
        t = int(parts[0])

        # Only keep data up to time 1800
        if t > 1800:
            break

        clock.append(t)
        walking.append(int(parts[1]))
        resting.append(int(parts[2]))
        collected_food.append(int(parts[3]))

# Plot
plt.figure()
plt.plot(clock, walking, label="Walking")
plt.plot(clock, resting, label="Resting")
plt.plot(clock, collected_food, label="Collected Food")

plt.xlabel("Clock")
plt.ylabel("Value")
plt.title("Activity Over Time - Replenishing Foraging (up to t = 1800)")
plt.legend()
plt.grid(True)

plt.show()
