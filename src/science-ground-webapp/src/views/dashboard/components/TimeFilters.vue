<template>
  <v-card outlined :class="!$vuetify.breakpoint.smAndDown ? 'ml-5 mb-2' : 'mb-2'" elevation="0" style="border: thin solid #999999;"   :width="!$vuetify.breakpoint.smAndDown ? '80%' :'100%'">
    <v-card-title outlined class="ma-0 pa-0">Filtri</v-card-title>
    <v-card outlined class="ma-2">
      <div class="ma-2">
        <v-dialog
          ref="dialogDateFrom"
          v-model="menuDialogFrom"
          :return-value.sync="dateFrom"
          persistent
          width="290px"
        >
          <template v-slot:activator="{ on, attrs }">
            <v-text-field
              v-model="dateFrom"
              label="Data di inizio"
              readonly
              v-bind="attrs"
              v-on="on"
            ></v-text-field>
          </template>
          <v-date-picker v-model="dateFrom" scrollable>
            <v-spacer></v-spacer>
            <v-btn text color="primary" @click="menuDialogFrom = false">
              Cancel
            </v-btn>
            <v-btn
              text
              color="primary"
              @click="$refs.dialogDateFrom.save(dateFrom)"
              :disabled="validDateInsert"
            >
              OK
            </v-btn>
          </v-date-picker>
        </v-dialog>
      </div>
      <div class="ma-2">
        <v-dialog
          ref="dialogTimeFrom"
          v-model="menuTimeFrom"
          :return-value.sync="timeFrom"
          persistent
          width="290px"
        >
          <template v-slot:activator="{ on, attrs }">
            <v-text-field
              v-model="timeFrom"
              label="Ora di inizio"
              readonly
              v-bind="attrs"
              v-on="on"
            ></v-text-field>
          </template>
          <v-time-picker v-if="menuTimeFrom" v-model="timeFrom" full-width format="24hr">
            <v-spacer></v-spacer>
            <v-btn text color="primary" @click="menuTimeFrom = false">
              Cancel
            </v-btn>
            <v-btn
              text
              color="primary"
              @click="$refs.dialogTimeFrom.save(timeFrom)"
              :disabled="validDateInsert"
            >
              OK
            </v-btn>
          </v-time-picker>
        </v-dialog>
      </div>
    </v-card>

    <v-card outlined class="ma-2">
      <div class="ma-2">
        <v-dialog
          ref="dialogDateTo"
          v-model="menuDateTo"
          :return-value.sync="dateTo"
          persistent
          width="290px"
        >
          <template v-slot:activator="{ on, attrs }">
            <v-text-field
              v-model="dateTo"
              label="Data di fine"
              readonly
              v-bind="attrs"
              v-on="on"
            ></v-text-field>
          </template>
          <v-date-picker v-model="dateTo" scrollable>
            <v-spacer></v-spacer>
            <v-btn text color="primary" @click="menuDateTo = false" >
              Cancel
            </v-btn>
            <v-btn
              text
              color="primary"
              @click="$refs.dialogDateTo.save(dateTo)"
               :disabled="validDateInsert"
            >
              OK
            </v-btn>
          </v-date-picker>
        </v-dialog>
      </div>
      <div class="ma-2">
        <v-dialog
          ref="dialogTimeTo"
          v-model="menuTimeTo"
          :return-value.sync="timeTo"
          persistent
          width="290px"
        >
          <template v-slot:activator="{ on, attrs }">
            <v-text-field
              v-model="timeTo"
              label="Ora di fine"
              readonly
              v-bind="attrs"
              v-on="on"
            ></v-text-field>
          </template>
          <v-time-picker v-if="menuTimeTo" v-model="timeTo" full-width format="24hr" > 
            <v-spacer></v-spacer>
            <v-btn text color="primary" @click="menuTimeTo = false">
              Cancel
            </v-btn>
            <v-btn
              text
              color="primary"
              @click="$refs.dialogTimeTo.save(timeTo)"
              :disabled="validDateInsert"
            >
              OK
            </v-btn>
          </v-time-picker>
        </v-dialog>
      </div>
    </v-card>
  </v-card>
</template>

<script>
import Vue from "vue";
import { EventBus } from "../../../main";
import moment from 'moment';

export default {
  name: "TimeFilters",
  components: {},
  data() {
    return {
      dateTo: new Date().toISOString().substr(0, 10),
      menuDateTo: false,
      dateFrom: new Date().toISOString().substr(0, 10),
      menuDialogFrom: false,

      timeTo: "23:59",
      menuTimeTo: false,
      timeFrom: "00:01",
      menuTimeFrom: false,
    };
  },
  computed:{
    validDateInsert(){
      return moment(this.dateFrom + ' ' + this.timeFrom, 'YYYY-MM-DD HH:mm') > moment(this.dateTo + ' ' + this.timeTo, 'YYYY-MM-DD HH:mm');
    },

  },

  watch:{
    menuDateTo(){
      if(!this.menuDateTo && !this.validDateInsert)
        this.changeFilter();
    },
    menuDialogFrom(){
      if(!this.menuDialogFrom && !this.validDateInsert)
        this.changeFilter();
    },
    menuTimeTo(){
      if(!this.menuTimeTo && !this.validDateInsert)
       this.changeFilter();
    },
    menuTimeFrom(){
       if(!this.menuTimeFrom && !this.validDateInsert)
         this.changeFilter();
    }
  },
  mounted() {
    let self = this;

    this.dateFrom = moment(this.dateTo, "YYYY-MM-DD").subtract(3, 'months').format("YYYY-MM-DD");
  },
  methods: {
    changeFilter() {
      EventBus.$emit("updateChart", {
        to: moment(this.dateTo + " " + this.timeTo, "YYYY-MM-DD HH:mm").format("YYYY-MM-DD HH:mm"),
        from: moment(this.dateFrom + " " + this.timeFrom, "YYYY-MM-DD HH:mm").format("YYYY-MM-DD HH:mm"),
      });
    }
  }
};
</script>
