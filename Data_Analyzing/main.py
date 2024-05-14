import os
from data_analysis import read_ping_data, calculate_delivery_rate, longest_consecutive_success, longest_burst_loss, calculate_packet_loss_correlation, find_min_max_rtt, plot_rtt_over_time, plot_rtt_distribution, plot_rtt_correlation, draw_conclusions

relative_path = "./Data_Analyzing"
absolute_path = os.path.abspath(relative_path)

if __name__ == "__main__":
    file_path = 'data.txt'
    timestamps, sequence_numbers, rtts = read_ping_data(absolute_path+file_path)
    
    delivery_rate = calculate_delivery_rate(sequence_numbers)
    longest_success = longest_consecutive_success(sequence_numbers)
    longest_loss = longest_burst_loss(sequence_numbers)
    prob_received_after_received, prob_received_after_lost = calculate_packet_loss_correlation(sequence_numbers)
    min_rtt, max_rtt = find_min_max_rtt(rtts)
    
    plot_rtt_over_time(timestamps, rtts)
    plot_rtt_distribution(rtts)
    plot_rtt_correlation(rtts)
    
    draw_conclusions(delivery_rate, longest_success, longest_loss, prob_received_after_received, prob_received_after_lost, min_rtt, max_rtt)
