import os
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import re
try:
    import loguru as logger
    logger_available = True
except ImportError:
    logger_available = False
    import logging
    logging.basicConfig(level=logging.INFO)
    logger = logging.getLogger(__name__)
from typing import List, Dict, Optional

# -------------------------------
# 全域設定
# -------------------------------
OUTPUT_FOLDER = "run/plot/image/"

line_colors = ["#000000", "#00EC00","#d62728","#0000C6", "#FF7F0E", "#9467bd", "#8c564b", "#e377c2", "#7f7f7f", "#bcbd22", "#17becf"]
# marker_types = ["h", "o", "^", "d", "D"]

plt_graph_settings: dict = { 
    "andy_theme": {
        "xtick.labelsize": 20,
        "ytick.labelsize": 20,
        "axes.labelsize": 20,
        "axes.titlesize": 20,
        "font.family": "Times New Roman",
        "mathtext.it": "Times New Roman:italic",
        "mathtext.fontset": "custom",
    },
    "ax1_settings": {
        "lw": 2.5,
        "linestyle": "-",
        "markersize": 12,
        "markeredgewidth": 2.5,
    },
    "ax1_tick_settings": {
        "direction": "in",
        "bottom": True,
        "top": True,
        "left": True,
        "right": True,
        "pad": 20,
    },
    "ax1_xaxis_label_coords": {
        "x": 0.45,
        "y": -0.27,
    },
    "ax1_yaxis_label_coords": {
        "x": -0.3,
        "y": 0.5,
    },
    "plt_xlabel_settings": {
        "fontsize": 32,
        "labelpad": 35,
    },
    "plt_ylabel_settings": {
        "fontsize": 32,
        "labelpad": 10,
    },
    "plt_xticks_settings": {
        "fontsize": 32,
    },
    "plt_yticks_settings": {
        "fontsize": 32,
    },
    "plt_leg_settings": {
        "loc": 10,
        "bbox_to_anchor": (0.4, 1.25),
        "prop": {"size": 32},
        "frameon": False,
        "labelspacing": 0.2,
        "handletextpad": 0.2,
        "handlelength": 1,
        "columnspacing": 0.2,
        "ncol": 3,
        "facecolor": "None",
    },
    "plt_leg_frame_settings": {
        "linewidth": 0.0,
    },
    "subplots_setting": {
        "figsize": (7, 6),
        "dpi": 600,
    },
    "subplots_adjust_settings": {
        "top": 0.75,
        "left": 0.25,
        "right": 0.95,
        "bottom": 0.25,
    },
}

# -------------------------------
# 讀取檔案並解析資料的類別
# -------------------------------
class AlgorithmData:
    def __init__(self, algorithm: str,  x_label: str, y_label: str, predefined_x: Optional[List[float]] = None):
        """
        @param algorithm: 算法名稱（如 "greedy"、"heuristic"、"mis"、"imp"）
        @param file_list: 此算法對應的檔案列表
        @param x_label: x 軸標籤
        @param y_label: y 軸標籤
        @param predefined_x: 預先定義的 x 軸值（若提供則使用這些值而非從檔名中提取）
        """
        self.algorithm = algorithm
        # self.file_list = file_list
        # string opname[6] = {"avg_obj", "avg_gen", "avg_cost", "avg_satis", "avg_power"};
        self.avg_file = f"data/res/avg/{algorithm}_avg_satis_avg.txt"
        self.x_label = x_label
        self.y_label = y_label
        self.x: List[float] = []  # 利用檔名中的數字作為 x 軸資料（例如執行編號）
        self.y: List[float] = []  # 從檔案中讀取的總生產速率
        self.output_filename = f"res_{algorithm}_user_rate.png"
        self.read_avg_file()

    def read_avg_file(self):
        try:
            with open(self.avg_file, "r", encoding="utf-8") as f:
                lines = f.readlines()
        except Exception as e:
            if logger_available:
                logger.logger.error(f"讀取檔案 {self.avg_file} 發生錯誤：{e}")
            else:
                logger.error(f"讀取檔案 {self.avg_file} 發生錯誤：{e}")
            return
        
        # Extract rates from each line of the average file
        rates = []
        for line in lines:
            try:
                # Parse the floating point number from each line
                rate = float(line.strip())
                rates.append(rate)
            except ValueError as e:
                if logger_available:
                    logger.logger.warning(f"無法解析行 '{line.strip()}': {e}")
                else:
                    logger.warning(f"無法解析行 '{line.strip()}': {e}")

        # Store the extracted rates
        self.y = rates

        # devide by 1e8
        # self.y = [rate / 1e6 for rate in self.y]

        # Note: x values will need to be set separately with set_x() method

    def read_files(self):
        for i, file in enumerate(self.file_list):
            try:
                with open(file, "r", encoding="utf-8") as f:
                    lines = f.readlines()
            except Exception as e:
                if logger_available:
                    logger.logger.error(f"讀取檔案 {file} 發生錯誤：{e}")
                else:
                    logger.error(f"讀取檔案 {file} 發生錯誤：{e}")
                continue
            
            rate = self.extract_rate(lines)
            self.y.append(rate)
    
    def set_x(self, x: List[float]):
        self.x = x
        if len(self.x) != len(self.y):
            if logger_available:
                logger.logger.error(f"x 軸的長度 {len(self.x)} 與 y 軸的長度 {len(self.y)} 不一致")
            else:
                logger.error(f"x 軸的長度 {len(self.x)} 與 y 軸的長度 {len(self.y)} 不一致")
            raise ValueError("x 軸與 y 軸的長度不一致")
        if len(self.x) == 0:
            if logger_available:
                logger.logger.error("x 軸的長度為 0，無法繪製圖表")
            else:
                logger.error("x 軸的長度為 0，無法繪製圖表")
            raise ValueError("x 軸的長度為 0，無法繪製圖表")

    def extract_rate(self, lines: List[str]) -> float:
        for line in lines:
            if line.startswith("Objective value"):
                parts = line.split(":")
                if len(parts) > 1:
                    try:
                        return float(parts[1].strip())
                    except Exception as e:
                        if logger_available:
                            logger.logger.error(f"解析生產速率錯誤：{e}")
                        else:
                            logger.error(f"解析生產速率錯誤：{e}")

        return 0.0

            

def plot_comparative_graph(dataset_ids=None, persat=None): # type: ignore

    os.makedirs(OUTPUT_FOLDER, exist_ok=True)
    
    # 根據檔名規則建立各算法的檔案列表
    filepath = os.path.join("data", "res")
    dataset_ids = [30 , 40, 50, 60, 70] 
    # cp_files = [os.path.join(filepath, f"greedy_cp_{i}_1.txt") for i in range(1, 6)]
    # w_files = [os.path.join(filepath, f"greedy_w_{i}_1.txt") for i in range(1, 6)]
    # obj_files = [os.path.join(filepath, f"greedy_obj_{i}_1.txt") for i in range(1, 6)]
    # a0_files = [os.path.join(filepath, f"a0_{i}_1.txt") for i in range(1, 6)]
    # sa_files = [os.path.join(filepath, f"SA_not_random_{i}_1.txt") for i in range(1, 6)]
    # ilp_files = [os.path.join(filepath, f"ILP_{i}_1.txt") for i in range(1, 6)]

    x_label = r"# UEs $\mid I\mid$"
    y_label = r"# Satisfied UEs"
    # Objective, Generation Rate, , , Objective ($\mathregular{\times 10^8}$
    # Tot. Wtd. Dem. Size 
    # 建立各算法的資料物件
    data_cp = AlgorithmData("greedy_cp", x_label=x_label, y_label=y_label)
    data_w = AlgorithmData("greedy_w", x_label=x_label, y_label=y_label)
    data_obj = AlgorithmData("greedy_obj", x_label=x_label, y_label=y_label)
    data_a0 = AlgorithmData("a0", x_label=x_label, y_label=y_label)
    data_sa = AlgorithmData("SA_not_random", x_label=x_label, y_label=y_label)
    data_ilp = AlgorithmData("ILP", x_label=x_label, y_label=y_label)

    print("data_cp.y:", data_cp.y)
    # print("data_w.y:", data_w.y)
    print("data_obj.y:", data_obj.y)
    print("data_a0.y:", data_a0.y)
    print("data_sa.y:", data_sa.y)
    # print("data_ilp.y:", data_ilp.y)

    # 設定 x 軸的值
    data_cp.set_x(dataset_ids)
    data_w.set_x(dataset_ids)
    data_obj.set_x(dataset_ids)
    data_a0.set_x(dataset_ids)
    data_sa.set_x(dataset_ids)
    data_ilp.set_x(dataset_ids)

    # 建立圖表並應用完整設定
    plt.rcParams.update(plt_graph_settings["andy_theme"])
    fig, ax = plt.subplots(**plt_graph_settings["subplots_setting"])

    # "h", "o", "^", "d"
    cp_settings = plt_graph_settings["ax1_settings"].copy()
    cp_settings.update({"marker": 'x', "markersize": 12, "markeredgewidth": 2.5})
    if len(data_cp.x) > 0 and len(data_cp.y) > 0:
        ax.plot(data_cp.x, data_cp.y,
                label="HRF",
                color=line_colors[0],
                **cp_settings)

    # w_settings = plt_graph_settings["ax1_settings"].copy()
    # w_settings.update({"marker": "o", "markersize": 10, "markeredgewidth": 2.5, "markerfacecolor": "none" })
    # if len(data_w.x) > 0 and len(data_w.y) > 0:
    #     ax.plot(data_w.x, data_w.y,
    #             label="HWF",
    #             color=line_colors[4],
    #             **w_settings)

    obj_settings = plt_graph_settings["ax1_settings"].copy()
    obj_settings.update({"marker": "d", "markersize": 12, "markeredgewidth": 2.5,"markerfacecolor": "none"})
    if len(data_obj.x) > 0 and len(data_obj.y) > 0:
        ax.plot(data_obj.x, data_obj.y,
                label="HVF",
                color=line_colors[1],
                **obj_settings)

    a0_settings = plt_graph_settings["ax1_settings"].copy()
    a0_settings.update({"marker": "^", "markersize": 12, "markeredgewidth": 2.5,"markerfacecolor": "none" })
    if len(data_a0.x) > 0 and len(data_a0.y) > 0:
        ax.plot(data_a0.x, data_a0.y,
                label="RELIC",
                color=line_colors[3],
                **a0_settings)
    
    sa_settings = plt_graph_settings["ax1_settings"].copy()
    sa_settings.update({"marker": "h", "markersize": 12, "markeredgewidth": 2.5,"markerfacecolor": "none" })
    if len(data_sa.x) > 0 and len(data_sa.y) > 0:
        ax.plot(data_sa.x, data_sa.y,
                label="SA",
                color=line_colors[2],
                **sa_settings)
        
    ilp_settings = plt_graph_settings["ax1_settings"].copy()
    ilp_settings.update({"marker": "D", "markersize": 12, "markeredgewidth": 2.5,"markerfacecolor": "none" })
    if len(data_ilp.x) > 0 and len(data_ilp.y) > 0:
        ax.plot(data_ilp.x, data_ilp.y,
                label="OPT",
                color=line_colors[5],
                **ilp_settings)

    # 設定刻度與軸標籤位置
    ax.tick_params(**plt_graph_settings["ax1_tick_settings"])
    ax.xaxis.set_label_coords(**plt_graph_settings["ax1_xaxis_label_coords"])
    ax.yaxis.set_label_coords(**plt_graph_settings["ax1_yaxis_label_coords"])
    

    # 設定 x 與 y 軸標籤和刻度字型
    ax.set_xlabel(x_label, **plt_graph_settings["plt_xlabel_settings"])
    ax.set_ylabel(y_label, **plt_graph_settings["plt_ylabel_settings"])
    plt.setp(ax.get_xticklabels(), **plt_graph_settings["plt_xticks_settings"])
    plt.setp(ax.get_yticklabels(), **plt_graph_settings["plt_yticks_settings"])
    
    # 設定 x 軸的刻度
    xticks = dataset_ids
    ax.set_xticks(xticks)
    
    # 設定 x 軸的範圍正好是第一個刻度到最後一個刻度
    # ax.set_xlim([xticks[0], xticks[-1]])
    
    # 設定 y 軸的刻度
    # yticks = [2, 4, 6, 8, 10]
    # ax.set_yticks(yticks)
    
    # 加入圖例與設定圖例邊框
    leg = ax.legend(**plt_graph_settings["plt_leg_settings"])
    leg.get_frame().set_linewidth(plt_graph_settings["plt_leg_frame_settings"]["linewidth"])

    # 調整圖表邊距並輸出圖檔
    plt.subplots_adjust(**plt_graph_settings["subplots_adjust_settings"])
    plt.tight_layout()

    filename = f"TMP_gen.png"
    output_file = os.path.join(OUTPUT_FOLDER, filename)
    plt.savefig(output_file)
    plt.close()
    
    if logger_available:
        logger.logger.info(f"比較圖已存檔於 {output_file}")
    else:
        logger.info(f"比較圖已存檔於 {output_file}")
    
    return output_file

if __name__ == "__main__":
    # 當直接執行此檔案時，使用預設方式繪圖
    plot_comparative_graph()