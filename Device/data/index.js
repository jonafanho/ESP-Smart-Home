const PORT_COUNT = 4;
const DEFAULT_RULES = [...Array(PORT_COUNT)].map((u, i) => []);
const CONDITIONS = {
	"time": {
		name: "The time of day is",
		min: 0,
		max: 1440,
		conversion: "time"
	},
	"dayOfWeek": {
		name: "The day of week is",
		values: ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
	},
	"temperature": {
		name: "The temperature is",
		min: -273,
		max: 1000,
		unit: "&deg;C"
	},
	"humidity": {
		name: "The humidity is",
		min: 0,
		max: 100,
		unit: "%"
	},
	"light": {
		name: "The light level is",
		min: 0,
		max: 100,
		unit: "%"
	},
	"proximity": {
		name: "Motion detector",
		values: ["People detected"]
	}
};

function setup() {
	"use strict";

	class MainScreen extends React.Component {

		constructor(props) {
			super(props);
			this.editOrSave = this.editOrSave.bind(this);
			this.state = {edit: false, rules: Object.assign({}, DEFAULT_RULES)};
		}

		editOrSave(event) {
			const isEdit = this.state.edit;
			if (isEdit) {
				fetch(`/settings?`, {
					method: "POST",
					body: "",
					headers: {
						"Content-type": "application/json; charset=UTF-8"
					}
				}).then(response => response.json()).then(data => {
				}).catch(error => {
				});
			}
			this.setState({edit: !isEdit});
		}

		render() {
			return (
				<div>
					<h1>Smart Home Device Configuration</h1>
					<table>
						<tbody>
						{[...Array(PORT_COUNT)].map((u, i) =>
							<Port key={i} rules={this.state.rules[i]} index={i} edit={this.state.edit}/>
						)}
						</tbody>
					</table>
					<br/>
					<input
						className="input_button"
						type="submit"
						value={this.state.edit ? "Save" : "Edit"}
						onClick={this.editOrSave}
					/>
				</div>
			);
		}
	}

	class Port extends React.Component {

		constructor(props) {
			super(props);
			this.addRule = this.addRule.bind(this);
			this.addCondition = this.addCondition.bind(this);
		}

		addRule(event) {
			this.props.rules.push([]);
			this.setState({});
		}

		addCondition(ruleIndex, event) {
			this.props.rules[ruleIndex].push(Object.assign({id: "time"}, CONDITIONS["time"]));
			this.setState({});
		}

		render() {
			const {rules, index, edit} = this.props;
			return (
				<tr>
					<td className="column_ports"><h2>Port {index + 1}</h2></td>
					<td className="column_conditions">
						{[...Array(rules.length)].map((u, ruleIndex) =>
							<div>
								{rules[ruleIndex].map(condition =>
									<Condition condition={condition} edit={edit}/>
								)}
								{edit ? <AddButton onClick={(event) => this.addCondition(ruleIndex, event)}/> : null}
							</div>
						)}
						{edit ? <AddButton onClick={this.addRule}/> : null}
					</td>
				</tr>
			);
		}
	}

	function AddButton(props) {
		return <div className="add_button" onClick={props["onClick"]}>+</div>;
	}

	class Condition extends React.Component {

		constructor(props) {
			super(props);
			this.changeCondition = this.changeCondition.bind(this);
			this.state = {};
		}

		changeCondition(event) {
			const {condition} = this.props;
			condition["id"] = event.target.value;
			this.setState({});
		}

		render() {
			const {condition, edit} = this.props;
			const details = CONDITIONS[condition["id"]];
			const isDiscrete = "values" in details;
			return (
				<div className={`condition ${condition["id"]}`}>
					{edit ?
						<select onChange={this.changeCondition}>
							{Object.keys(CONDITIONS).map(conditionKey =>
								<option
									key={conditionKey}
									selected={condition["id"] === conditionKey}
									value={conditionKey}
								>{CONDITIONS[conditionKey]["name"]}</option>
							)}
						</select> :
						details["name"]
					}
					<br/>
					{isDiscrete ? [...Array(details["values"].length)].map((u, index) =>
							<div>
								<label>
									<input key={index} className="input_checkbox" type="checkbox"/>
									{details["values"][index]}
								</label>
								<br/>
							</div>
						) :
						<select>
							<option value="lessThan">less than</option>
							<option value="greaterThan">greater than</option>
							<option value="inRange">between</option>
							<option value="outsideRange">not between</option>
						</select>
					}
				</div>
			);
		}
	}

	if (window.location.pathname !== "/") {
		window.location.pathname = "/";
	}
	ReactDOM.render(<MainScreen/>, document.querySelector("#react-root"));
}

setup();