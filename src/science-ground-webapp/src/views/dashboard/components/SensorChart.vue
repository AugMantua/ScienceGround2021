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
import Vue from "vue";
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
      options: {
        chart: {
          id: "vuechart-example",
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
      self.getTerrariunDatas(value.from, value.to);
    });
  },

  methods: {
    getTerrariunDatas(from, to) {
      let self = this;
      self.loading = true;

      Vue.axios
        .get("/data/measures/get?TerrariumID=" + self.terrariumId + "&From="+ from +"&To=" + to + "&SensorID=" + self.sensorDatas.ID)
        .then((res) => {
          console.log(res);

          let temp = [];

          if (res.data == null) {
            self.series = [
              {
                data: [],
              },
            ];
            self.loading = false;
            return;
          }

          res.data.forEach((element) => {
            temp.push({
              x: element.Timestamp,
              y: element.Value,
            });
          });

          self.series = [
            {
              data: temp,
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
                text: self.sensorDatas.TypeOfMeasure +
                " (" +
                self.sensorDatas.Extra_data +
                ")",
              },
            },
          });
          self.loading = false;
        })
        .catch((err) => {
          self.loading = false;
        });
    },
  },
};
</script>