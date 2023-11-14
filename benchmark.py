import matplotlib.pyplot as plt
import os

def fetch_results(depth=1):
    output = os.popen(f"./p4 -d {depth} -f -s").read()
    tags = output.splitlines()[0].split(",")
    values = output.splitlines()[1].split(",")

    data = {}
    for i, tag in enumerate(tags):
        data[tag] = float(values[i])

    return data

def benchmark(n, keys, orders, depth=1):
    tmp_results = {}
    for _ in range(n):
        r = fetch_results(depth)
        for key in keys:
            if not key in tmp_results:
                tmp_results[key] = []
            tmp_results[key].append(r[key])

    opt_results = {}
    for i, key in enumerate(keys):
        if orders[i] == "max":
            opt_results[key] = max(tmp_results[key])
        else:
            opt_results[key] = min(tmp_results[key])

    return opt_results

if __name__ == "__main__":
    times = []
    hit_rates = []

    fig, axs = plt.subplots(2)

    depths = range(1, 15, 2)
    for depth in depths:
        data = benchmark(10, ("time", "hit_rate"), ("min", "max"), depth)
        times.append(data["time"])
        hit_rates.append(data["hit_rate"] * 100)

    axs[0].plot(depths, times, "bo:")
    # axs[0].set_title("Temps d'exécution en fonction de la profondeur")
    axs[0].set_xlabel("Profondeur")
    axs[0].set_ylabel("Temps (s)")

    axs[1].plot(depths, hit_rates, "ro:")
    # axs[1].set_title("Taux de hit en fonction de la profondeur")
    axs[1].set_xlabel("Profondeur")
    axs[1].set_ylabel("Taux de hit (%)")

    plt.show()
