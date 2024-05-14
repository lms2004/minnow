import pandas as pd
import matplotlib.pyplot as plt
import datetime
from collections import defaultdict

# 读取data.txt文件并解析数据
def read_ping_data(file_path):
    timestamps, sequence_numbers, rtts = [], [], []
    with open(file_path) as file:
        for line in file:
            parts = line.split()
            if len(parts) < 8:
                continue
            timestamp = int(parts[0][1:-1])
            seq_number = int(parts[4].split('=')[1])
            rtt = float(parts[6].split('=')[1])
            timestamps.append(datetime.datetime.fromtimestamp(timestamp))
            sequence_numbers.append(seq_number)
            rtts.append(rtt)
    return timestamps, sequence_numbers, rtts

# 计算总体交付率
def calculate_delivery_rate(sequence_numbers):
    total_sent = sequence_numbers[-1] - sequence_numbers[0] + 1
    total_received = len(sequence_numbers)
    delivery_rate = total_received / total_sent
    return delivery_rate

# 找出data.txt文件中连续回复的最长序列
def longest_consecutive_success(sequence_numbers):
    longest_success, current_success = 0, 0
    previous_seq = sequence_numbers[0] - 1
    for seq in sequence_numbers:
        if seq == previous_seq + 1:
            current_success += 1
            if current_success > longest_success:
                longest_success = current_success
        else:
            current_success = 1
        previous_seq = seq
    return longest_success

# 找出data.txt文件中连续丢包的最长序列
def longest_burst_loss(sequence_numbers):
    longest_loss, current_loss = 0, 0
    previous_seq = sequence_numbers[0] - 1
    for seq in sequence_numbers:
        if seq != previous_seq + 1:
            current_loss = seq - previous_seq - 1
            if current_loss > longest_loss:
                longest_loss = current_loss
        previous_seq = seq
    return longest_loss

# 计算丢包的相关性
def calculate_packet_loss_correlation(sequence_numbers):
    received_after_received = 0
    received_after_lost = 0
    total_received = 0
    total_lost = 0
    previous_received = False
    previous_seq = sequence_numbers[0] - 1
    for seq in sequence_numbers:
        if seq == previous_seq + 1:
            if previous_received:
                received_after_received += 1
            else:
                received_after_lost += 1
            previous_received = True
            total_received += 1
        else:
            previous_received = False
            total_lost += seq - previous_seq - 1
        previous_seq = seq
    prob_received_after_received = received_after_received / total_received if total_received > 0 else 0
    prob_received_after_lost = received_after_lost / total_lost if total_lost > 0 else 0
    return prob_received_after_received, prob_received_after_lost

# 找出data.txt文件中的最小RTT值和最大RTT值
def find_min_max_rtt(rtts):
    return min(rtts), max(rtts)

# 绘制RTT随时间变化的图
def plot_rtt_over_time(timestamps, rtts):
    plt.figure(figsize=(10, 5))
    plt.plot(timestamps, rtts)
    plt.xlabel('时间')
    plt.ylabel('RTT（毫秒）')
    plt.title('RTT随时间变化的图')
    plt.show()

# 绘制RTT分布的直方图或累积分布函数（CDF）
def plot_rtt_distribution(rtts):
    plt.figure(figsize=(10, 5))
    plt.hist(rtts, bins=50, cumulative=True, density=True, histtype='step', label='CDF')
    plt.xlabel('RTT（毫秒）')
    plt.ylabel('概率')
    plt.title('RTT的CDF')
    plt.legend()
    plt.show()

# 绘制连续ping的RTT相关性图
def plot_rtt_correlation(rtts):
    rtt_pairs = [(rtts[i], rtts[i+1]) for i in range(len(rtts)-1)]
    plt.figure(figsize=(10, 5))
    plt.scatter(*zip(*rtt_pairs), alpha=0.5, edgecolors='none')
    plt.xlabel('ping#N的RTT（毫秒）')
    plt.ylabel('ping#N+1的RTT（毫秒）')
    plt.title('连续ping的RTT相关性')
    plt.show()

# 分析数据并得出结论
def draw_conclusions(delivery_rate, longest_success, longest_loss, prob_received_after_received, prob_received_after_lost, min_rtt, max_rtt):
    print("结论:")
    print(f"1. 总体交付率为: {delivery_rate:.2%}")
    print(f"2. 最长连续成功ping的字符串为: {longest_success}")
    print(f"3. 最长的丢包突发为: {longest_loss}")
    print(f"4. 收到回复后下一次收到回复的概率为: {prob_received_after_received:.2%}")
    print(f"5. 未收到回复后下一次收到回复的概率为: {prob_received_after_lost:.2%}")
    print(f"6. 最小RTT为: {min_rtt} ms")
    print(f"7. 最大RTT为: {max_rtt} ms")
    print("通过这些分析，我们可以看到网络路径的稳定性和可靠性。网络延迟在时间上有波动，但整体交付率较高。")
