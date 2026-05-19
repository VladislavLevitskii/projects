import pandas as pd
from statsmodels.stats.proportion import proportions_ztest
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.patches as mpatches

def plot_winrates(map_names, pro_rates, casual_rates, p_values, alpha=0.05):
    x = np.arange(len(map_names))
    width = 0.35

    fig, ax = plt.subplots(figsize=(12, 7))

    ax.axhspan(49, 51, color='gray', alpha=0.2)
    ax.axhline(y=50, color='black', linestyle='--', alpha=0.5)

    for i in range(len(map_names)):
        is_significant = p_values[i] < alpha
        pro_color = '#2a9d8f' if is_significant else '#b2d8d3'
        casual_color = '#e76f51' if is_significant else '#f4c4b8'

        ax.bar(x[i] - width/2, pro_rates[i], width, color=pro_color)
        ax.bar(x[i] + width/2, casual_rates[i], width, color=casual_color)

    ax.set_ylabel('CT Winrate (%)', fontweight='bold')
    ax.set_title('CT Side Advantage: Pro vs. Casual (Highlighting Significant Differences)', fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(map_names, rotation=45)
    ax.set_ylim(40, 60)

    legend_elements = [
        mpatches.Patch(color='#2a9d8f', label='Pro (H0 Rejected)'),
        mpatches.Patch(color='#e76f51', label='Casual (H0 Rejected)'),
        mpatches.Patch(color='#b2d8d3', label='Pro (Failed to Reject)'),
        mpatches.Patch(color='#f4c4b8', label='Casual (Failed to Reject)'),
        plt.Line2D([0], [0], color='black', linestyle='--', label='50% Balance'),
        mpatches.Patch(color='gray', alpha=0.2, label='49%-51% Balance Zone')
    ]
    ax.legend(handles=legend_elements, loc='upper right')

    ax.grid(axis='y', linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.show()

def load_data(pro_path, casual_path):
    df_pro = pd.read_csv(pro_path)
    df_casual = pd.read_csv(casual_path)
    df_rounds = df_casual.drop_duplicates(subset=['file', 'round'])
    return df_pro, df_rounds

def calculate_map_stats(df_pro, df_casual, map_name):
    target_map_pro = map_name
    target_map_casual = f"de_{map_name.lower()}"

    pro_map_data = df_pro[df_pro['_map'] == target_map_pro]
    casual_map_data = df_casual[df_casual['map'] == target_map_casual]

    if len(pro_map_data) == 0 or len(casual_map_data) == 0:
        return None

    pro_ct_wins = (pro_map_data['ct_1'] + pro_map_data['ct_2']).sum()
    pro_total_rounds = pro_ct_wins + (pro_map_data['t_1'] + pro_map_data['t_2']).sum()

    casual_ct_wins = len(casual_map_data[casual_map_data['winner_side'] == 'CounterTerrorist'])
    casual_total_rounds = len(casual_map_data)

    if pro_total_rounds == 0 or casual_total_rounds == 0:
        return None

    return (pro_ct_wins, pro_total_rounds), (casual_ct_wins, casual_total_rounds)

def run_test_for_map(map_name, pro_stats, casual_stats, alpha=0.05):
    pro_ct_wins, pro_total = pro_stats
    casual_ct_wins, casual_total = casual_stats

    successes = [pro_ct_wins, casual_ct_wins]
    observations = [pro_total, casual_total]

    z_stat, p_value = proportions_ztest(count=successes, nobs=observations)

    print(f"--- Hypothesis Test Results for {map_name} ---")
    print(f"Pro CT Winrate:    {(pro_ct_wins / pro_total) * 100:.2f}% (from {pro_total} rounds)")
    print(f"Casual CT Winrate: {(casual_ct_wins / casual_total) * 100:.2f}% (from {casual_total} rounds)")
    print(f"Z-Statistic:       {z_stat:.4f}")
    print(f"P-Value:           {p_value:.6f}")
    
    if p_value < alpha:
        print("Conclusion: Reject the Null Hypothesis. (Significant difference)")
    else:
        print("Conclusion: Fail to reject the Null Hypothesis. (No significant difference)")
    
    print("-" * 50)
    return p_value

def main():
    pro_path = "./data/pro_data.csv"
    casual_path = "./data/casual_data.csv"
    
    df_pro, df_casual = load_data(pro_path, casual_path)
    
    maps_to_test = [
        'Mirage', 'Inferno', 'Overpass', 'Nuke', 
        'Vertigo', 'Dust2', 'Train', 'Cache', 'Cobblestone'
    ]

    plot_maps = []
    plot_pro = []
    plot_casual = []
    plot_p_values = []

    for map_name in maps_to_test:
        stats = calculate_map_stats(df_pro, df_casual, map_name)
        
        if stats:
            pro_stats, casual_stats = stats
            p_val = run_test_for_map(map_name, pro_stats, casual_stats)

            plot_maps.append(map_name)
            plot_pro.append((pro_stats[0] / pro_stats[1]) * 100)
            plot_casual.append((casual_stats[0] / casual_stats[1]) * 100)
            plot_p_values.append(p_val)

    if plot_maps:
        plot_winrates(plot_maps, plot_pro, plot_casual, plot_p_values)

if __name__ == "__main__":
    main()