<template>
  <v-card elevation="0" outlined :loading="loading">
    <apexchart
      type="line"
      :options="options"
      :series="series"
      ref="sensorChart"
    ></apexchart>
  </v-card>
</template>

<script>
import VueApexCharts from "vue-apexcharts";
import { EventBus } from "../../../main";

export default {
  name: "SensorChart",
  components: {
    apexchart: VueApexCharts,
  },
  props: ["sensorDatas", "terrariumId", "dateTimeFilters"],
  data() {
    return {
      loading: true,
      toFilter: "",

      options: {
        chart: {
          id: "vuechart-example",
        },
        animations: {
          enabled: true,
          easing: "linear",
          dynamicAnimation: {
            speed: 1000,
          },
        },
        xaxis: {
          labels: {
            show: false,
          },
        },
        yaxis: {
          title: {
            text: "",
          },
        },
      },
      series: [
        {
          name: "series-1",
          data: [],
        },
      ],
    };
  },

  mounted() {
    let self = this;

    EventBus.$on("updateChart", (value) => {
      self.reloadAllData(value);
    });
    EventBus.$on("filterUpdated", (value) => {
      if (value.onlyLast != undefined || value.onlyLast) {
        self.reloadAllData({ data: null, key: null }); // when enter in live mode reset chart
      }
    });
  },
  beforeDestroy() {
    EventBus.$off("updateChart");
    EventBus.$off("filterUpdated");
  },
  methods: {
    reloadAllData(res) {
      let self = this;

      if (res.data == null) {
        self.$refs.sensorChart.updateSeries([
          {
            data: [],
          },
        ]);
        self.loading = false;
        return;
      }

      if (res.key != null && res.key != self.sensorDatas.ID) {
        self.loading = false;
        return;
      }

      // update series on mode live
      if (res.liveMode) {
        if (
          self.series[0].data[self.series[0].data.length - 1].x != res.data[0].x
        ) {
          self.series[0].data.push(res.data[0]);

          self.$refs.sensorChart.updateSeries([
            {
              data: self.series[0].data,
            },
          ]);
        }
        return;
      }

      self.series = [
        {
          data: res.data,
          name:
            self.sensorDatas.TypeOfMeasure +
            " (" +
            self.sensorDatas.Extra_data +
            ")",
        },
      ];
      self.$refs.sensorChart.updateOptions({
        yaxis: {
          title: {
            text:
              self.sensorDatas.TypeOfMeasure +
              " (" +
              self.sensorDatas.Extra_data +
              ")",
          },
        },
      });
      self.loading = false;
    },
  },
};
</script>